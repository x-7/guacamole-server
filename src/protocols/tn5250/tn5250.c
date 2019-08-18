/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "config.h"
#include "common/recording.h"
#include "tn5250.h"
#include "terminal/terminal.h"

#include <guacamole/client.h>
#include <guacamole/protocol.h>
#include <libtelnet.h>

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

/**
 * Support levels for various telnet options, required for connection
 * negotiation by telnet_init(), part of libtelnet.  The TN5250 specification
 * requires at EOR, BINARY, and TTYPE.
 */
static const telnet_telopt_t __telnet_options[] = {
    { TELNET_TELOPT_ECHO,        TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_TTYPE,       TELNET_WILL, TELNET_DO   },
    { TELNET_TELOPT_COMPRESS2,   TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_MSSP,        TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_NAWS,        TELNET_WONT, TELNET_DONT },
    { TELNET_TELOPT_NEW_ENVIRON, TELNET_WILL, TELNET_DONT },
    { TELNET_TELOPT_EOR,         TELNET_WILL, TELNET_DO   },
    { TELNET_TELOPT_BINARY,      TELNET_WILL, TELNET_DO   },
    { -1, 0, 0 }
};

/**
 * ***Should be abstracted to common telnet support.***
 * Write the entire buffer given to the specified file descriptor, retrying
 * the write automatically if necessary. This function will return a value
 * not equal to the buffer's size iff an error occurs which prevents all
 * future writes.
 *
 * @param fd The file descriptor to write to.
 * @param buffer The buffer to write.
 * @param size The number of bytes from the buffer to write.
 */
static int __guac_telnet_write_all(int fd, const char* buffer, int size) {

    int remaining = size;
    while (remaining > 0) {

        /* Attempt to write data */
        int ret_val = write(fd, buffer, remaining);
        if (ret_val <= 0)
            return -1;

        /* If successful, continue with what data remains (if any) */
        remaining -= ret_val;
        buffer += ret_val;

    }

    return size;

}

/**
 * Event handler, as defined by libtelnet. This function is passed to
 * telnet_init() and will be called for every event fired by libtelnet,
 * including feature enable/disable and receipt/transmission of data.
 */
static void __guac_tn5250_event_handler(telnet_t* telnet, telnet_event_t* event, void* data) {

    guac_client* client = (guac_client*) data;
    guac_tn5250_client* tn5250_client = (guac_tn5250_client*) client->data;
    guac_tn5250_settings* settings = tn5250_client->settings;

    switch (event->type) {

        /* Terminal output received */
        case TELNET_EV_DATA:
            __guac_tn5250_recv_sna_packet(client, event);
            // guac_terminal_write(tn5250_client->term, event->data.buffer, event->data.size);
            break;

        /* Data destined for remote end */
        case TELNET_EV_SEND:
            if (__guac_tn5250_send_sna_packet(telnet, event))
                guac_client_stop(client);
            if (__guac_tn5250_write_all(tn5250_client->socket_fd, event->data.buffer, event->data.size)
                    != event->data.size)
                guac_client_stop(client);
            break;

        /* Remote feature enabled */
        case TELNET_EV_WILL:
            guac_client_log(client, GUAC_LOG_DEBUG, "Received TELNET_EV_WILL "
                    "for option %d", event->neg.telopt);
            if (event->neg.telopt == TELNET_TELOPT_ECHO)
                tn5250_client->echo_enabled = 0; /* Disable local echo, as remote will echo */
            break;

        /* Remote feature disabled */
        case TELNET_EV_WONT:
            guac_client_log(client, GUAC_LOG_DEBUG, "Received TELNET_EV_WONT "
                    "for option %d", event->neg.telopt);
            if (event->neg.telopt == TELNET_TELOPT_ECHO)
                tn5250_client->echo_enabled = 1; /* Enable local echo, as remote won't echo */
            break;

        /* Local feature enable */
        case TELNET_EV_DO:
            guac_client_log(client, GUAC_LOG_DEBUG, "Received TELNET_EV_DO "
                    "for option %d", event->neg.telopt);
            break;

        /* Terminal type request */
        case TELNET_EV_TTYPE:
            guac_client_log(client, GUAC_LOG_DEBUG, "Received TELNET_EV_TTYPE "
                    "for command %d", event->ttype.cmd);
            if (event->ttype.cmd == TELNET_TTYPE_SEND)
                tn5250_ttype_is(tn5250_client->telnet, __guac_tn5250_terminals[settings->terminal_type].terminal);
            break;

        /* Environment request */
        case TELNET_EV_ENVIRON:

            guac_client_log(client, GUAC_LOG_DEBUG, "Received TELNET_EV_ENVIRON "
                    "for size %d", event->environ.size);
            
            /* Only send USER if entire environment was requested */
            if (event->environ.size == 0)
                guac_tn5250_send_user(telnet, settings->username);

            break;

        /* Connection warnings */
        case TELNET_EV_WARNING:
            guac_client_log(client, GUAC_LOG_WARNING, "%s", event->error.msg);
            break;

        /* Connection errors */
        case TELNET_EV_ERROR:
            guac_client_abort(client, GUAC_PROTOCOL_STATUS_UPSTREAM_ERROR,
                    "Telnet connection closing with error: %s", event->error.msg);
            break;

        /* Ignore other events */
        default:
            break;

    }

}

/**
 * Input thread, started by the main telnet client thread. This thread
 * continuously reads from the terminal's STDIN and transfers all read
 * data to the telnet connection.
 *
 * @param data The current guac_client instance.
 * @return Always NULL.
 */
static void* __guac_tn5250_input_thread(void* data) {

    guac_client* client = (guac_client*) data;
    guac_tn5250_client* tn5250_client = (guac_tn5250_client*) client->data;

    char buffer[8192];
    int bytes_read;

    /* Write all data read */
    while ((bytes_read = guac_terminal_read_stdin(tn5250_client->term, buffer, sizeof(buffer))) > 0) {
        telnet_send(tn5250_client->telnet, buffer, bytes_read);
        if (tn5250_client->echo_enabled)
            guac_terminal_write(tn5250_client->term, buffer, bytes_read);
    }

    return NULL;

}

/**
 * ***Abstract to common telnet code.***
 * Connects to the telnet server specified within the data associated
 * with the given guac_client, which will have been populated by
 * guac_client_init.
 *
 * @return The connected telnet instance, if successful, or NULL if the
 *         connection fails for any reason.
 */
static telnet_t* __guac_tn5250_create_session(guac_client* client) {

    int retval;

    int fd;
    struct addrinfo* addresses;
    struct addrinfo* current_address;

    char connected_address[1024];
    char connected_port[64];

    guac_tn5250_client* tn5250_client = (guac_tn5250_client*) client->data;
    guac_tn5250_settings* settings = tn5250_client->settings;

    struct addrinfo hints = {
        .ai_family   = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP
    };

    /* Get socket */
    fd = socket(AF_INET, SOCK_STREAM, 0);

    /* Get addresses connection */
    if ((retval = getaddrinfo(settings->hostname, settings->port,
                    &hints, &addresses))) {
        guac_client_abort(client, GUAC_PROTOCOL_STATUS_SERVER_ERROR, "Error parsing given address or port: %s",
                gai_strerror(retval));
        return NULL;

    }

    /* Attempt connection to each address until success */
    current_address = addresses;
    while (current_address != NULL) {

        int retval;

        /* Resolve hostname */
        if ((retval = getnameinfo(current_address->ai_addr,
                current_address->ai_addrlen,
                connected_address, sizeof(connected_address),
                connected_port, sizeof(connected_port),
                NI_NUMERICHOST | NI_NUMERICSERV)))
            guac_client_log(client, GUAC_LOG_DEBUG, "Unable to resolve host: %s", gai_strerror(retval));

        /* Connect */
        if (connect(fd, current_address->ai_addr,
                        current_address->ai_addrlen) == 0) {

            guac_client_log(client, GUAC_LOG_DEBUG, "Successfully connected to "
                    "host %s, port %s", connected_address, connected_port);

            /* Done if successful connect */
            break;

        }

        /* Otherwise log information regarding bind failure */
        else
            guac_client_log(client, GUAC_LOG_DEBUG, "Unable to connect to "
                    "host %s, port %s: %s",
                    connected_address, connected_port, strerror(errno));

        current_address = current_address->ai_next;

    }

    /* If unable to connect to anything, fail */
    if (current_address == NULL) {
        guac_client_abort(client, GUAC_PROTOCOL_STATUS_UPSTREAM_NOT_FOUND,
                "Unable to connect to any addresses.");
        return NULL;
    }

    /* Free addrinfo */
    freeaddrinfo(addresses);

    /* Open telnet session */
    telnet_t* telnet = telnet_init(__telnet_options, __guac_telnet_event_handler, 0, client);
    if (telnet == NULL) {
        guac_client_abort(client, GUAC_PROTOCOL_STATUS_SERVER_ERROR, "Telnet client allocation failed.");
        return NULL;
    }

    /* Save file descriptor */
    tn5250_client->socket_fd = fd;

    return telnet;

}

/**
 * Sends a 16-bit value over the given telnet connection with the byte order
 * required by the telnet protocol.
 *
 * @param telnet The telnet connection to use.
 * @param value The value to send.
 */
static void __guac_tn5250_send_uint16(telnet_t* telnet, uint16_t value) {

    unsigned char buffer[2];
    buffer[0] = (value >> 8) & 0xFF;
    buffer[1] =  value       & 0xFF;

    telnet_send(telnet, (char*) buffer, 2);

}

/**
 * Sends an 8-bit value over the given telnet connection.
 *
 * @param telnet The telnet connection to use.
 * 
 * @param value The value to send.
 */
static void __guac_tn5250_send_uint8(telnet_t* telnet, uint8_t value) {
    telnet_send(telnet, (char*) (&value), 1);
}

void guac_tn5250_send_user(telnet_t* telnet, const char* username) {

    /* IAC SB NEW-ENVIRON IS */
    telnet_begin_sb(telnet, TELNET_TELOPT_NEW_ENVIRON);
    __guac_tn5250_send_uint8(telnet, TELNET_ENVIRON_IS);

    /* Only send username if defined */
    if (username != NULL) {

        /* VAR "USER" */
        __guac_tn5250_send_uint8(telnet, TELNET_ENVIRON_VAR);
        telnet_send(telnet, "USER", 4);

        /* VALUE username */
        __guac_tn5250_send_uint8(telnet, TELNET_ENVIRON_VALUE);
        telnet_send(telnet, username, strlen(username));

    }

    /* IAC SE */
    telnet_finish_sb(telnet);

}

/**
 * Waits for data on the given file descriptor for up to one second. The
 * return value is identical to that of select(): 0 on timeout, < 0 on
 * error, and > 0 on success.
 *
 * @param socket_fd The file descriptor to wait for.
 * 
 * @return A value greater than zero on success, zero on timeout, and
 *         less than zero on error.
 */
static int __guac_tn5250_wait(int socket_fd) {

    /* Build array of file descriptors */
    struct pollfd fds[] = {{
        .fd      = socket_fd,
        .events  = POLLIN,
        .revents = 0,
    }};

    /* Wait for one second */
    return poll(fds, 1, 1000);

}

void* guac_tn5250_client_thread(void* data) {

    guac_client* client = (guac_client*) data;
    guac_tn5250_client* tn5250_client = (guac_tn5250_client*) client->data;
    guac_tn5250_settings* settings = tn5250_client->settings;

    pthread_t input_thread;
    char buffer[8192];
    int wait_result;

    /* Set up screen recording, if requested */
    if (settings->recording_path != NULL) {
        tn5250_client->recording = guac_common_recording_create(client,
                settings->recording_path,
                settings->recording_name,
                settings->create_recording_path,
                !settings->recording_exclude_output,
                !settings->recording_exclude_mouse,
                settings->recording_include_keys);
    }

    /* Calculate required terminal size based on type. */
    
    /* Create terminal */
    tn5250_client->term = guac_terminal_create(client,
            tn5250_client->clipboard, settings->disable_copy,
            settings->max_scrollback, settings->font_name, settings->font_size,
            settings->resolution, settings->width, settings->height,
            settings->color_scheme, settings->backspace);

    /* Fail if terminal init failed */
    if (tn5250_client->term == NULL) {
        guac_client_abort(client, GUAC_PROTOCOL_STATUS_SERVER_ERROR,
                "Terminal initialization failed");
        return NULL;
    }
    
    /* Fix terminal width/height */
    tn5250_client->term.term_width = __guac_tn5250_terminals[settings->terminal_type].cols;
    tn5250_client->term.term_height = __guac_tn5250_terminals[settings->terminal_type].rows;

    /* Set up typescript, if requested */
    if (settings->typescript_path != NULL) {
        guac_terminal_create_typescript(tn5250_client->term,
                settings->typescript_path,
                settings->typescript_name,
                settings->create_typescript_path);
    }

    /* Open telnet session */
    tn5250_client->telnet = __guac_tn5250_create_session(client);
    if (tn5250_client->telnet == NULL) {
        /* Already aborted within __guac_tn5250_create_session() */
        return NULL;
    }

    /* Logged in */
    guac_client_log(client, GUAC_LOG_INFO, "TN5250 connection successful.");

    /* Allow terminal to render */
    guac_terminal_start(tn5250_client->term);

    /* Start input thread */
    if (pthread_create(&(input_thread), NULL, __guac_tn5250_input_thread, (void*) client)) {
        guac_client_abort(client, GUAC_PROTOCOL_STATUS_SERVER_ERROR, "Unable to start input thread");
        return NULL;
    }

    /* While data available, write to terminal */
    while ((wait_result = __guac_tn5250_wait(tn5250_client->socket_fd)) >= 0) {

        /* Resume waiting of no data available */
        if (wait_result == 0)
            continue;

        int bytes_read = read(tn5250_client->socket_fd, buffer, sizeof(buffer));
        if (bytes_read <= 0)
            break;

        telnet_recv(tn5250_client->telnet, buffer, bytes_read);

    }

    /* Kill client and Wait for input thread to die */
    guac_client_stop(client);
    pthread_join(input_thread, NULL);

    guac_client_log(client, GUAC_LOG_INFO, "TN5250 connection ended.");
    return NULL;

}

void __guac_tn5250_send_sna_packet(void* data, tn5250_flags flags,
        unsigned char opcode, char* data) {
    
    /**
     * Things to do, here:
     * - Put the TN5250 header on
     * - Set any flags
     * - Set the Opcode
     * - Write data
     * - Write EOR
     */
    
    
}

void __guac_tn5250_recv_sna_packet(guac_client* client, telnet_event_t* event) {
    
    /**
     * Things to do, here:
     * - Check for TN5250 header
     * - Take off flags and process
     * - Examine Opcode and handle (switch)
     * - Read data and handle it (while)
     * - Look for EOR (while)
     */
    
    /* Look for TN5250 header - abort if not found. */
    
    /* Grab flags */
    
    /* Process Opcode */
    switch (opcode) {
        
        case OPCODE_NOOP:
            
            break;
            
        case OPCODE_INVITE:
            
            break;
            
        case OPCODE_OUTPUT:
            
            break;
            
        case OPCODE_PUT_GET:
            
            break;
            
        case OPCODE_SAVE_SCREEN:
            
            break;
            
        case OPCODE_RESTORE_SCREEN:
            
            break;
            
        case OPCODE_READ_IMMEDIATE:
            
            break:
            
        case OPCODE_READ_SCREEN:
            
            break;
            
        case OPCODE_CANCEL_INVITE:
            
            break;
            
        case OPCODE_MSG_ON:
            
            break;
            
        case OPCODE_MSG_OFF:
            
            break;
            
        default:
            
    }
    
    /* Process data until we get the EOR option */
    while (packet[i] != TELNET_TELOPT_EOR) {
        
    }
    
}
