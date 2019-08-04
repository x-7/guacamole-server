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

#ifndef GUAC_TN5250_SETTINGS_H
#define GUAC_TN5250_SETTINGS_H

#include "config.h"

#include <guacamole/user.h>

#include <sys/types.h>
#include <regex.h>
#include <stdbool.h>

/**
 * The name of the font to use for the terminal if no name is specified.
 */
#define GUAC_TN5250_DEFAULT_FONT_NAME "monospace" 

/**
 * The size of the font to use for the terminal if no font size is specified,
 * in points.
 */
#define GUAC_TN5250_DEFAULT_FONT_SIZE 12

/**
 * The port to connect to when initiating a 5250 telnet connection, if no other
 * port is specified.
 */
#define GUAC_TN5250_DEFAULT_PORT "23"

/**
 * The port to connect to when initiating a 5250 telnet connection over SSL,
 * if no other port is specified.
 */
#define GUAC_TN5250_DEFAULT_SSL_PORT "992"

/**
 * The filename to use for the typescript, if not specified.
 */
#define GUAC_TN5250_DEFAULT_TYPESCRIPT_NAME "typescript" 

/**
 * The filename to use for the screen recording, if not specified.
 */
#define GUAC_TN5250_DEFAULT_RECORDING_NAME "recording"

/**
 * The default maximum scrollback size in rows.
 */
#define GUAC_TN5250_DEFAULT_MAX_SCROLLBACK 1000

/**
 * The available terminal types for TN5250 connections.  These are defined
 * in RFC-1205, Section 2.
 */
typedef enum guac_tn5250_terminal_types {
    
    /**
     * IBM-3179-2, 24 x 80 color display
     */
    IBM_3179_2,

    /**
     * IBM-3180-2, 27 x 132 monochrome display
     */
    IBM_3180_2,

    /**
     * IBM-3196-A1, 24 x 80 monochrome display
     */
    IBM_3196_A1,

    /**
     * IBM-3477-FC, 27 x 132 color display
     */
    IBM_3477_FC,

    /**
     * IBM-3477-FG, 27 x 132 monochrome display
     */
    IBM_3477_FG,

    /**
     * IBM-5251-11, 24 x 80 monochrome display
     */
    IBM_5251_11,

    /**
     * IBM-5291-1, 24 x 80 monochrome display
     */
    IBM_5291_1,

    /**
     * IBM-5292-2, 24 x 80 color display
     */
    IBM_5292_2,

    /**
     * IBM-5555-B01, 24 x 80 Double-Byte Character Set (DBCS)
     */
    IBM_5555_B01,

    /**
     * IBM-5555-C01, 24 x 80 Double-Byte Character Set color display
     */
    IBM_5555_C01
    
} guac_tn5250_terminal_type;

/**
 * Data structure to store all of the characteristics of various types of
 * 5250-compatible terminals.
 */
typedef struct __guac_tn5250_terminal_params {
    
    /**
     * The type of terminal, as defined in RFC-1205
     */
    char* terminal;
    
    /**
     * The number of rows in the terminal
     */
    int rows;
    
    /**
     * The number of columns in the terminal
     */
    int cols;
    
    /**
     * True if the terminal supports color, false if only monochrome.
     */
    bool color;
    
} __guac_tn5250_terminal_params;

/**
 * An array of all of the possible terminal types, including the ENUM value,
 * the height (in rows) and width (in columns), and whether or not the terminal
 * supports color (true if color, false if monochrome).
 */
__guac_tn5250_terminal_params __guac_tn5250_terminals[] = {
    {"IBM_3179_2",   24, 80,  true },
    {"IBM_3180_2",   27, 132, false},
    {"IBM_3196_A1",  24, 80,  false},
    {"IBM_3477_FC",  27, 132, true },
    {"IBM_3477_FG",  27, 132, false},
    {"IBM_5251_11",  24, 80,  false},
    {"IBM_5291_1",   24, 80,  false},
    {"IBM_5292_2",   24, 80,  true },
    {"IBM_5555_B01", 24, 80,  false},
    {"IBM_5555_C01", 24, 80,  true },
    {NULL,         -1, -1,  false}
};

/**
 * Settings for the TN5250 connection. The values for this structure are parsed
 * from the arguments given during the Guacamole protocol handshake using the
 * guac_tn5250_parse_args() function.
 */
typedef struct guac_tn5250_settings {

    /**
     * The hostname of the TN5250 server to connect to.
     */
    char* hostname;

    /**
     * The port of the TN5250 server to connect to.
     */
    char* port;
    
    /**
     * Whether or not to use SSL.
     */
    bool ssl;
    
    /**
     * Whether or not to use enhanced TN5250 mode (RFC2877)
     */
    bool enhanced;

    /**
     * Whether this connection is read-only, and user input should be dropped.
     */
    bool read_only;

    /**
     * The maximum size of the scrollback buffer in rows.
     */
    int max_scrollback;

    /**
     * The name of the font to use for display rendering.
     */
    char* font_name;

    /**
     * The size of the font to use, in points.
     */
    int font_size;

    /**
     * The name of the color scheme to use.
     */
    char* color_scheme; 

    /**
     * The desired width of the terminal display, in pixels.
     */
    int width;

    /**
     * The desired height of the terminal display, in pixels.
     */
    int height;

    /**
     * The desired screen resolution, in DPI.
     */
    int resolution;

    /**
     * Whether outbound clipboard access should be blocked. If set, it will not
     * be possible to copy data from the terminal to the client using the
     * clipboard.
     */
    bool disable_copy;

    /**
     * Whether inbound clipboard access should be blocked. If set, it will not
     * be possible to paste data from the client to the terminal using the
     * clipboard.
     */
    bool disable_paste;

    /**
     * The path in which the typescript should be saved, if enabled. If no
     * typescript should be saved, this will be NULL.
     */
    char* typescript_path;

    /**
     * The filename to use for the typescript, if enabled.
     */
    char* typescript_name;

    /**
     * Whether the typescript path should be automatically created if it does
     * not already exist.
     */
    bool create_typescript_path;

    /**
     * The path in which the screen recording should be saved, if enabled. If
     * no screen recording should be saved, this will be NULL.
     */
    char* recording_path;

    /**
     * The filename to use for the screen recording, if enabled.
     */
    char* recording_name;

    /**
     * Whether the screen recording path should be automatically created if it
     * does not already exist.
     */
    bool create_recording_path;

    /**
     * Whether output which is broadcast to each connected client (graphics,
     * streams, etc.) should NOT be included in the session recording. Output
     * is included by default, as it is necessary for any recording which must
     * later be viewable as video.
     */
    bool recording_exclude_output;

    /**
     * Whether changes to mouse state, such as position and buttons pressed or
     * released, should NOT be included in the session recording. Mouse state
     * is included by default, as it is necessary for the mouse cursor to be
     * rendered in any resulting video.
     */
    bool recording_exclude_mouse;

    /**
     * Whether keys pressed and released should be included in the session
     * recording. Key events are NOT included by default within the recording,
     * as doing so has privacy and security implications.  Including key events
     * may be necessary in certain auditing contexts, but should only be done
     * with caution. Key events can easily contain sensitive information, such
     * as passwords, credit card numbers, etc.
     */
    bool recording_include_keys;

    /**
     * The ASCII code, as an integer, that the 5250 client will use when the
     * backspace key is pressed.  By default, this is 127, ASCII delete, if
     * not specified in the client settings.
     */
    int backspace;

    /**
     * The terminal emulator type that is passed to the remote system.
     */
    guac_tn5250_terminal_type terminal_type;

} guac_tn5250_settings;

/**
 * Parses all given args, storing them in a newly-allocated settings object. If
 * the args fail to parse, NULL is returned.
 *
 * @param user
 *     The user who submitted the given arguments while joining the
 *     connection.
 *
 * @param argc
 *     The number of arguments within the argv array.
 *
 * @param argv
 *     The values of all arguments provided by the user.
 *
 * @return
 *     A newly-allocated settings object which must be freed with
 *     guac_tn5250_settings_free() when no longer needed. If the arguments fail
 *     to parse, NULL is returned.
 */
guac_tn5250_settings* guac_tn5250_parse_args(guac_user* user,
        int argc, const char** argv);

/**
 * Frees the regex pointed to by the given pointer, assigning the value NULL to
 * that pointer once the regex is freed. If the pointer already contains NULL,
 * this function has no effect.
 *
 * @param regex
 *     The address of the pointer to the regex that should be freed.
 */
void guac_tn5250_regex_free(regex_t** regex);

/**
 * Frees the given guac_tn5250_settings object, having been previously
 * allocated via guac_tn5250_parse_args().
 *
 * @param settings
 *     The settings object to free.
 */
void guac_tn5250_settings_free(guac_tn5250_settings* settings);

/**
 * NULL-terminated array of accepted client args.
 */
extern const char* GUAC_TN5250_CLIENT_ARGS[];

#endif

