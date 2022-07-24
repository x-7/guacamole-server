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

#ifndef GUAC_LIB5250_H
#define GUAC_LIB5250_H

#include "config.h"
#include "tn5250.h"

#include <guacamole/client.h>

#include <stdint.h>

/**
 * The General Data Stream SNA packet type.
 */
#define GUAC_LIB5250_SNA_GDS 0x12a0

/**
 * An enum that represents all of the available terminal types according to
 * RFC1205, the specification that documents the Telnet 5250 (TN5250)
 * standard.
 */
typedef enum guac_lib5250_terminal {

    /* 24 x 80 Double-Byte Character Set color display */
    GUAC_LIB5250_TERMINAL_IBM_5555_C01,

    /* 24 x 80 Double-Byte Character Set (DBCS) */
    GUAC_LIB5250_TERMINAL_IBM_5555_B01,

    /* 27 x 132 color display */
    GUAC_LIB5250_TERMINAL_IBM_3477_FC,

    /* 27 x 132 monochrome display */
    GUAC_LIB5250_TERMINAL_IBM_3477_FG,

    /* 27 x 132 monochrome display */
    GUAC_LIB5250_TERMINAL_IBM_3180_2,

    /* 24 x 80 color display */
    GUAC_LIB5250_TERMINAL_IBM_3179_2,

    /* 24 x 80 monochrome display */
    GUAC_LIB5250_TERMINAL_IBM_3196_A1,

    /* 24 x 80 color display */
    GUAC_LIB5250_TERMINAL_IBM_5292_2,

    /* 24 x 80 monochrome display */
    GUAC_LIB5250_TERMINAL_IBM_5291_1,

    /* 24 x 80 monochrome display */
    GUAC_LIB5250_TERMINAL_IBM_5251_11

} guac_lib5250_terminal;

/**
 * A data type that captures a terminal type, from the guac_lib5250_terminal
 * enum, and the capabilities associated with that terminal.
 */
typedef struct guac_lib5250_terminal_cap {

    /* The name of the terminal. */
    guac_lib5250_terminal name;

    /* The number of rows supported on the terminal. */
    unsigned int rows;

    /* The number of columns supports on the terminal. */
    unsigned int cols;

    /* 1 if the terminal supports colors, 0 if it is monochrome. */
    unsigned int color;

    /* The number of bytes-per-character the terminal supports. This must
     * be either 1 or 2. */
    unsigned int charbytes;

} guac_lib5250_terminal_cap;

/**
 * A list of the possible TN5250 opcodes, which can be sent to the client to
 * direct it to do something with the terminal. These are documented in RFC1205.
 */
typedef enum guac_lib5250_opcode {
    
    /* No Operation */
    GUAC_LIB5250_OPCODE_NOOP = 0x00,

    /* Invite terminal */
    GUAC_LIB5250_OPCODE_INVITE = 0x01,

    /* Output only */
    GUAC_LIB5250_OPCODE_OUTPUT = 0x02,

    /* Put/Get */
    GUAC_LIB5250_OPCODE_PUTGET = 0x03,

    /* Save screen */
    GUAC_LIB5250_OPCODE_SAVESCREEN = 0x04,

    /* Restore screen */
    GUAC_LIB5250_OPCODE_RESTORESCREEN = 0x05,

    /* Read immediate */
    GUAC_LIB5250_OPCODE_READIMMEDIATE = 0x06,

    /* Read screen */
    GUAC_LIB5250_OPCODE_READSCREEN = 0x08,

    /* Cancel invite */
    GUAC_LIB5250_OPCODE_CANCELINVITE = 0x0A,

    /* Message light on */
    GUAC_LIB5250_OPCODE_MSGON = 0x0B,

    /* Message light off */
    GUAC_LIB5250_OPCODE_MSGOFF = 0x0C

} guac_lib5250_opcode;

/**
 * A data type that defines the structure of the packet for 5250 communication
 * across telnet. The structure of this packet is based off of the definition
 * in RFC1205.
 */
typedef struct guac_lib5250_packet {
   
    /* The length of the entire logical record. */
    uint16_t len;

    /* The SNA record type. This should always be 0x12A0, which is the
     * General Data Stream record. */
    uint16_t type;

    /* Empty space, should be zerod. */
    uint16_t reserved;

    /* The length of the variable portion of the header, in octets. According
     * to the RFC, this should always be 0x04. */
    uint8_t varlen;

    /* Flags associated with the header, as documented in the RFC. The
     * bits are as follows:
     * Bit 0: ERR - Data stream output error.
     * Bit 1: ATN - Indicates Attention key was pressed.
     * Bit 2-4: Reserved, zeros.
     * Bit 5: SRQ - System Request key has been pressed.
     * Bit 6: TRQ - Test Request key has been pressed.
     * Bit 7: HLP - Help in Error State, with actual help code sent as data after
     *              the header.
     */
    uint8_t flags;

    /* At present, the remaining 8 bits of flags are not used at all. As old
     * as RFC1205 is, I doubt that's likely to change. These bits should be
     * all zeroes.
     */
    uint8_t reserved_flags;

    /* The operation code, as requested by the sender. Valid values for this
     * are in the gauc_lib5250_opcode enum.
     */
    uint8_t opcode;

    /* What remains is the data in the packet, which should consume the
     * remainder of the space that the header did not take up. The packet
     * should also be terminated with a Telnet IAC EOR record, 0xffef.
     */
    void* data;

} guac_lib5250_packet __attribute__ ((aligned));

/**
 * Parses the packet that libtelnet has received and then takes action
 * based on the contents of the packet.
 *
 * @param data
 *     The data buffer delivered by libtelnet, which should be binary telnet
 *     data that is the actual TN5250 protocol.
 *
 * @param client
 *     The Guacamole client associated with this connection.
 *
 * @return
 *     Zero if the data was successfully processed, non-zero otherwise.
 */
int guac_lib5250_process_packet(const char* data, guac_client* client);

#endif // GUAC_LIB5250_H