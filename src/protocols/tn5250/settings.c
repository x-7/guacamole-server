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

#include "settings.h"

#include <guacamole/user.h>

#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Client plugin arguments */
const char* GUAC_TN5250_CLIENT_ARGS[] = {
    "hostname",
    "port",
    "ssl",
    "enhanced",
    "font-name",
    "font-size",
    "color-scheme",
    "typescript-path",
    "typescript-name",
    "create-typescript-path",
    "recording-path",
    "recording-name",
    "recording-exclude-output",
    "recording-exclude-mouse",
    "recording-include-keys",
    "create-recording-path",
    "read-only",
    "backspace",
    "terminal-type",
    "scrollback",
    "disable-copy",
    "disable-paste",
    NULL
};

enum TN5250_ARGS_IDX {
    
    /**
     * The hostname to connect to. Required.
     */
    IDX_HOSTNAME,

    /**
     * The port to connect to. Optional.
     */
    IDX_PORT,
    
    /**
     * Whether or not to use SSL.  Optional.
     */
    IDX_SSL,
    
    /**
     * Whether or not to use RFC2877 enhanced TN5250.  Optional.
     */
    IDX_ENHANCED,
    
    /**
     * The name of the font to use within the terminal.
     */
    IDX_FONT_NAME,

    /**
     * The size of the font to use within the terminal, in points.
     */
    IDX_FONT_SIZE,

    /**
     * The color scheme to use, as a series of semicolon-separated color-value
     * pairs: "background: <color>", "foreground: <color>", or
     * "color<n>: <color>", where <n> is a number from 0 to 255, and <color> is
     * "color<n>" or an X11 color code (e.g. "aqua" or "rgb:12/34/56").
     * The color scheme can also be one of the special values: "black-white",
     * "white-black", "gray-black", or "green-black".
     */
    IDX_COLOR_SCHEME,

    /**
     * The full absolute path to the directory in which typescripts should be
     * written.
     */
    IDX_TYPESCRIPT_PATH,

    /**
     * The name that should be given to typescripts which are written in the
     * given path. Each typescript will consist of two files: "NAME" and
     * "NAME.timing".
     */
    IDX_TYPESCRIPT_NAME,

    /**
     * Whether the specified typescript path should automatically be created
     * if it does not yet exist.
     */
    IDX_CREATE_TYPESCRIPT_PATH,

    /**
     * The full absolute path to the directory in which screen recordings
     * should be written.
     */
    IDX_RECORDING_PATH,

    /**
     * The name that should be given to screen recordings which are written in
     * the given path.
     */
    IDX_RECORDING_NAME,

    /**
     * Whether output which is broadcast to each connected client (graphics,
     * streams, etc.) should NOT be included in the session recording. Output
     * is included by default, as it is necessary for any recording which must
     * later be viewable as video.
     */
    IDX_RECORDING_EXCLUDE_OUTPUT,

    /**
     * Whether changes to mouse state, such as position and buttons pressed or
     * released, should NOT be included in the session recording. Mouse state
     * is included by default, as it is necessary for the mouse cursor to be
     * rendered in any resulting video.
     */
    IDX_RECORDING_EXCLUDE_MOUSE,

    /**
     * Whether keys pressed and released should be included in the session
     * recording. Key events are NOT included by default within the recording,
     * as doing so has privacy and security implications.  Including key events
     * may be necessary in certain auditing contexts, but should only be done
     * with caution. Key events can easily contain sensitive information, such
     * as passwords, credit card numbers, etc.
     */
    IDX_RECORDING_INCLUDE_KEYS,

    /**
     * Whether the specified screen recording path should automatically be
     * created if it does not yet exist.
     */
    IDX_CREATE_RECORDING_PATH,

    /**
     * "true" if this connection should be read-only (user input should be
     * dropped), "false" or blank otherwise.
     */
    IDX_READ_ONLY,

    /**
     * ASCII code, as an integer to use for the backspace key, or 127
     * if not specified.
     */
    IDX_BACKSPACE,

    /**
     * The terminal emulator type that is passed to the remote system (e.g.
     * "xterm" or "xterm-256color"). "linux" is used if unspecified.
     */
    IDX_TERMINAL_TYPE,

    /**
     * The maximum size of the scrollback buffer in rows.
     */
    IDX_SCROLLBACK,

    /**
     * Whether outbound clipboard access should be blocked. If set to "true",
     * it will not be possible to copy data from the terminal to the client
     * using the clipboard. By default, clipboard access is not blocked.
     */
    IDX_DISABLE_COPY,

    /**
     * Whether inbound clipboard access should be blocked. If set to "true", it
     * will not be possible to paste data from the client to the terminal using
     * the clipboard. By default, clipboard access is not blocked.
     */
    IDX_DISABLE_PASTE,

    TN5250_ARGS_COUNT
};

guac_tn5250_settings* guac_tn5250_parse_args(guac_user* user,
        int argc, const char** argv) {

    /* Validate arg count */
    if (argc != TN5250_ARGS_COUNT) {
        guac_user_log(user, GUAC_LOG_WARNING, "Incorrect number of connection "
                "parameters provided: expected %i, got %i.",
                TN5250_ARGS_COUNT, argc);
        return NULL;
    }

    guac_tn5250_settings* settings = calloc(1, sizeof(guac_tn5250_settings));

    /* Read hostname */
    settings->hostname =
        guac_user_parse_args_string(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_HOSTNAME, "");

    /* Read-only mode */
    settings->read_only =
        guac_user_parse_args_boolean(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_READ_ONLY, false);

    /* Read maximum scrollback size */
    settings->max_scrollback =
        guac_user_parse_args_int(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_SCROLLBACK, GUAC_TN5250_DEFAULT_MAX_SCROLLBACK);

    /* Read font name */
    settings->font_name =
        guac_user_parse_args_string(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_FONT_NAME, GUAC_TN5250_DEFAULT_FONT_NAME);

    /* Read font size */
    settings->font_size =
        guac_user_parse_args_int(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_FONT_SIZE, GUAC_TN5250_DEFAULT_FONT_SIZE);

    /* Copy requested color scheme */
    settings->color_scheme =
        guac_user_parse_args_string(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_COLOR_SCHEME, "");

    /* Pull width/height/resolution directly from user */
    settings->width      = user->info.optimal_width;
    settings->height     = user->info.optimal_height;
    settings->resolution = user->info.optimal_resolution;

    /* Read SSL */
    settings->ssl =
        guac_user_parse_args_boolean(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_SSL, false);
    
    int defualt_port = GUAC_TN5250_DEFAULT_PORT;
    
    if (settings->ssl)
        default_port = GUAC_TN5250_DEFAULT_SSL_PORT;
    
    /* Read port */
    settings->port =
        guac_user_parse_args_string(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_PORT, defualt_port);
    
    /* Enhanced mode */
    settings->enhanced =
        guac_user_parse_args_boolean(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_ENHANCED, false);

    /* Read typescript path */
    settings->typescript_path =
        guac_user_parse_args_string(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_TYPESCRIPT_PATH, NULL);

    /* Read typescript name */
    settings->typescript_name =
        guac_user_parse_args_string(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_TYPESCRIPT_NAME, GUAC_TN5250_DEFAULT_TYPESCRIPT_NAME);

    /* Parse path creation flag */
    settings->create_typescript_path =
        guac_user_parse_args_boolean(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_CREATE_TYPESCRIPT_PATH, false);

    /* Read recording path */
    settings->recording_path =
        guac_user_parse_args_string(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_RECORDING_PATH, NULL);

    /* Read recording name */
    settings->recording_name =
        guac_user_parse_args_string(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_RECORDING_NAME, GUAC_TN5250_DEFAULT_RECORDING_NAME);

    /* Parse output exclusion flag */
    settings->recording_exclude_output =
        guac_user_parse_args_boolean(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_RECORDING_EXCLUDE_OUTPUT, false);

    /* Parse mouse exclusion flag */
    settings->recording_exclude_mouse =
        guac_user_parse_args_boolean(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_RECORDING_EXCLUDE_MOUSE, false);

    /* Parse key event inclusion flag */
    settings->recording_include_keys =
        guac_user_parse_args_boolean(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_RECORDING_INCLUDE_KEYS, false);

    /* Parse path creation flag */
    settings->create_recording_path =
        guac_user_parse_args_boolean(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_CREATE_RECORDING_PATH, false);

    /* Parse backspace key code */
    settings->backspace =
        guac_user_parse_args_int(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_BACKSPACE, 127);

    /* Read terminal emulator type. */
    char* terminal_type = guac_user_parse_args_string(user,
            GUAC_TN5250_CLIENT_ARGS, argv, IDX_TERMINAL_TYPE, "5251-11");
    
    if (strcmp(terminal_type, "3179-2") == 0)
        settings->terminal_type = IBM_3179_2;
    else if (strcmp(terminal_type, "3180-2") == 0)
        settings->terminal_type = IBM_3180_2;
    else if (strcmp(terminal_type, "3196-a1") == 0)
        settings->terminal_type = IBM_3196_A1;
    else if (strcmp(terminal_type, "3477-fc") == 0)
        settings->terminal_type = IBM_3477_FC;
    else if (strcmp(terminal_type, "3477-fg") == 0)
        settings->terminal_type = IBM_3477_FG;
    else if (strcmp(terminal_type, "5251-11") == 0)
        settings->terminal_type = IBM_5251_11;
    else if (strcmp(terminal_type, "5291-1") == 0)
        settings->terminal_type = IBM_5291_1;
    else if (strcmp(terminal_type, "5292-2") == 0)
        settings->terminal_type = IBM_5292_2;
    else if (strcmp(terminal_type, "5555-b01") == 0)
        settings->terminal_type = IBM_5555_B01;
    else if (strcmp(terminal_type, "5555-c01") == 0)
        settings->terminal_type = IBM_5555_C01;
    else {
        guac_user_log(user, GUAC_LOG_WARNING,
                "Invalid terminal type %s, defaulting to 5251-11",
                terminal_type);
        settings->terminal_type = IBM_5251_11;
    }
    free(terminal_type);
    

    /* Parse clipboard copy disable flag */
    settings->disable_copy =
        guac_user_parse_args_boolean(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_DISABLE_COPY, false);

    /* Parse clipboard paste disable flag */
    settings->disable_paste =
        guac_user_parse_args_boolean(user, GUAC_TN5250_CLIENT_ARGS, argv,
                IDX_DISABLE_PASTE, false);

    /* Parsing was successful */
    return settings;

}

void guac_tn5250_settings_free(guac_tn5250_settings* settings) {

    /* Free network connection information */
    free(settings->hostname);
    free(settings->port);

    /* Free display preferences */
    free(settings->font_name);
    free(settings->color_scheme);

    /* Free typescript settings */
    free(settings->typescript_name);
    free(settings->typescript_path);

    /* Free screen recording settings */
    free(settings->recording_name);
    free(settings->recording_path);

    /* Free terminal emulator type. */
    free(settings->terminal_type);

    /* Free overall structure */
    free(settings);

}

