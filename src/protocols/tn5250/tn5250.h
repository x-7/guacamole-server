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

#ifndef GUAC_TN5250_H
#define GUAC_TN5250_H

#include "config.h"
#include "common/clipboard.h"
#include "common/recording.h"
#include "settings.h"
#include "sna.h"
#include "terminal/terminal.h"

#include <libtelnet.h>

#include <stdint.h>

/**
 * Telnet-specific client data.
 */
typedef struct guac_tn5250_client {

    /**
     * TN5250 connection settings.
     */
    guac_tn5250_settings* settings;

    /**
     * The tn5250 client thread.
     */
    pthread_t client_thread;

    /**
     * The file descriptor of the socket connected to the tn5250 server,
     * or -1 if no connection has been established.
     */
    int socket_fd;

    /**
     * telnet connection, used by the tn5250 client thread.
     */
    telnet_t* telnet;

    /**
     * Whether window size should be sent when the window is resized.
     */
    int naws_enabled;

    /**
     * Whether all user input should be automatically echoed to the
     * terminal.
     */
    int echo_enabled;

    /**
     * The current clipboard contents.
     */
    guac_common_clipboard* clipboard;

    /**
     * The terminal which will render all output from the tn5250 client.
     */
    guac_terminal* term;

    /**
     * The in-progress session recording, or NULL if no recording is in
     * progress.
     */
    guac_common_recording* recording;

} guac_tn5250_client;

/**
 * Possible Opcodes that can be sent by the mainframe or the client.
 */
unsigned char tn5250_opcodes[] = {
    OPCODE_NOOP, /* No operation */
    OPCODE_INVITE, /* Invite */
    OPCODE_OUTPUT, /* Output only */
    OPCODE_PUT_GET, /* Put/Get */
    OPCODE_SAVE_SCREEN, /* Save screen */
    OPCODE_RESTORE_SCREEN, /* Restore screen */
    OPCODE_READ_IMMEDIATE, /* Read immediate */
    0x07, /* Reserved */
    OPCODE_READ_SCREEN, /* Read screen */
    0x09, /* Reserved */
    OPCODE_CANCEL_INVITE, /* Cancel invite */
    OPCODE_MSG_ON, /* Turn on message light */
    OPCODE_MSG_OFF, /* Turn off message light */
    NULL
} 

/**
 * Main tn5250 client thread, handling transfer of tn5250 output to STDOUT.
 * 
 * @param data
 *     The client data associated with this thread.
 */
void* guac_tn5250_client_thread(void* data);

/**
 * Sends the given username by setting the remote USER environment variable
 * using the tn5250 NEW-ENVIRON option.
 * 
 * @param telnet
 *     The telnet connection to send the USER variable to.
 * 
 * @param username
 *     The username to send.
 */
void guac_tn5250_send_user(telnet_t* telnet, const char* username);

/**
 * Sends the given data in TN5250 mode, creating the necessary packet
 * structure over the telnet connection to talk to the system.
 * 
 * @param telnet
 *     The telnet client sending the packet.
 * 
 * @param sna_flags
 *     Any flags that should be set in the packet.
 * 
 * @param opcode
 *     Any opcode that should be sent to the mainframe.
 * 
 * @param data
 *     Data that should be sent to the mainframe.
 */
void __guac_tn5250_send_sna_packet(telnet_t* telnet, sna_flags flags,
        unsigned char opcode, char* data);

/**
 * Handles a received SNA packet, processing flags and opcodes, and then writing
 * any output to the terminal.
 * 
 * @param client
 *     The Guacamole Client that is receiving the packet.
 * 
 * @param event
 *     The Telnet event that is being processed.
 */
void __guac_tn5250_recv_sna_packet(guac_client* client, telnet_event_t* event);

#endif

