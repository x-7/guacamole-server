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
#include "input.h"
#include "terminal/terminal.h"
#include "tn5250.h"

#include <guacamole/client.h>
#include <guacamole/user.h>
#include <libtelnet.h>

#include <regex.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int guac_tn5250_user_mouse_handler(guac_user* user, int x, int y, int mask) {

    guac_client* client = user->client;
    guac_tn5250_client* tn5250_client = (guac_tn5250_client*) client->data;
    guac_tn5250_settings* settings = tn5250_client->settings;
    guac_terminal* term = tn5250_client->term;

    /* Skip if terminal not yet ready */
    if (term == NULL)
        return 0;

    /* Report mouse position within recording */
    if (tn5250_client->recording != NULL)
        guac_common_recording_report_mouse(tn5250_client->recording, x, y,
                mask);

    /* Send mouse if not searching for password or username */
    if (settings->password_regex == NULL && settings->username_regex == NULL)
        guac_terminal_send_mouse(term, user, x, y, mask);

    return 0;

}

int guac_tn5250_user_key_handler(guac_user* user, int keysym, int pressed) {

    guac_client* client = user->client;
    guac_tn5250_client* tn5250_client = (guac_tn5250_client*) client->data;
    guac_tn5250_settings* settings = tn5250_client->settings;
    guac_terminal* term = tn5250_client->term;

    /* Report key state within recording */
    if (tn5250_client->recording != NULL)
        guac_common_recording_report_key(tn5250_client->recording,
                keysym, pressed);

    /* Skip if terminal not yet ready */
    if (term == NULL)
        return 0;

    /* Intercept and handle Pause / Break / Ctrl+0 as "IAC BRK" */
    if (pressed && (
                keysym == 0xFF13                  /* Pause */
             || keysym == 0xFF6B                  /* Break */
             || (term->mod_ctrl && keysym == '0') /* Ctrl + 0 */
       )) {

        /* Send IAC BRK */
        telnet_iac(tn5250_client->telnet, TELNET_BREAK);

        return 0;
    }

    /* Send key */
    guac_terminal_send_key(term, keysym, pressed);

    return 0;

}

int guac_tn5250_user_size_handler(guac_user* user, int width, int height) {

    /* Get terminal */
    guac_client* client = user->client;
    guac_tn5250_client* tn5250_client = (guac_tn5250_client*) client->data;
    guac_terminal* terminal = tn5250_client->term;

    /* Skip if terminal not yet ready */
    if (terminal == NULL)
        return 0;

    /* Resize terminal */
    guac_terminal_resize(terminal, width, height);

    /* Update terminal window size if connected */
    if (tn5250_client->telnet != NULL && tn5250_client->naws_enabled)
        guac_tn5250_send_naws(tn5250_client->telnet, terminal->term_width,
                terminal->term_height);

    return 0;
}

