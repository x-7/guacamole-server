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
#include "settings.h"
#include "terminal/terminal.h"

#include <guacamole/recording.h>
#include <libtelnet.h>

#include <stdint.h>

/**
 * TN5250-specific client data.
 */
typedef struct guac_tn5250_client {

    /**
     * Telnet connection settings.
     */
    guac_tn5250_settings* settings;

    /**
     * The TN5250 client thread.
     */
    pthread_t client_thread;

    /**
     * The file descriptor of the socket connected to the tn5250 server,
     * or -1 if no connection has been established.
     */
    int socket_fd;

    /**
     * TN5250 connection, used by the telnet client thread.
     */
    telnet_t* tn5250;

    /**
     * Whether all user input should be automatically echoed to the
     * terminal.
     */
    int echo_enabled;

    /**
     * The terminal which will render all output from the tn5250 client.
     */
    guac_terminal* term;

    /**
     * The in-progress session recording, or NULL if no recording is in
     * progress.
     */
    guac_recording* recording;

    /**
     * Indicates whether or not the TN5250 client is now in binary mode and
     * subsequent data should be binary.
     */
    int binary_mode;

} guac_tn5250_client;

/**
 * Main tn5250 client thread, handling transfer of TN5250 output to STDOUT.
 */
void* guac_tn5250_client_thread(void* data);

/**
 * Sends the given username by setting the remote USER environment variable
 * using the telnet NEW-ENVIRON option.
 */
void guac_tn5250_send_user(telnet_t* tn5250, const char* username);

#endif

