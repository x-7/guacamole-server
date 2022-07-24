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
#include "lib5250.h"
#include "tn5250.h"

#include <guacamole/client.h>

#include <arpa/inet.h>

int guac_lib5250_process_packet(const char* data, guac_client* client) {

    guac_client_log(client, GUAC_LOG_DEBUG, "%s: Processing expected TN5250 SNA data.", __func__);

    guac_lib5250_packet* packet = (guac_lib5250_packet*) data;

    if (ntohs(packet->type) != GUAC_LIB5250_SNA_GDS) {
        guac_client_abort(client, GUAC_PROTOCOL_STATUS_UPSTREAM_ERROR,
                "%s: Unexpected SNA packet typing parsing TN5250 packet: 0x%x",
                __func__, ntohs(packet->type));
        return 1;
    }

    switch (packet->opcode) {

        case GUAC_LIB5250_OPCODE_NOOP:
            guac_client_log(client, GUAC_LOG_TRACE, "%s: Received opcode no-op.", __func__);
            break;

        case GUAC_LIB5250_OPCODE_INVITE:
            guac_client_log(client, GUAC_LOG_TRACE, "%s: Received opcode invite.", __func__);
            break;

        case GUAC_LIB5250_OPCODE_OUTPUT:
            guac_client_log(client, GUAC_LOG_TRACE, "%s: Received opcode output.", __func__);
            break;

        case GUAC_LIB5250_OPCODE_PUTGET:
            guac_client_log(client, GUAC_LOG_TRACE, "%s: Received opcode put/get.", __func__);
            break;

        case GUAC_LIB5250_OPCODE_SAVESCREEN:
            guac_client_log(client, GUAC_LOG_TRACE, "%s: Received opcode save screen.", __func__);
            break;

        case GUAC_LIB5250_OPCODE_RESTORESCREEN:
            guac_client_log(client, GUAC_LOG_TRACE, "%s: Received opcode restore screen.", __func__);
            break;

        case GUAC_LIB5250_OPCODE_READIMMEDIATE:
            guac_client_log(client, GUAC_LOG_TRACE, "%s: Received opcode read immediate.", __func__);
            break;

        case GUAC_LIB5250_OPCODE_READSCREEN:
            guac_client_log(client, GUAC_LOG_TRACE, "%s: Received opcode read screen.", __func__);
            break;

        case GUAC_LIB5250_OPCODE_CANCELINVITE:
            guac_client_log(client, GUAC_LOG_TRACE, "%s: Received opcode cancel invite.", __func__);
            break;

        case GUAC_LIB5250_OPCODE_MSGON:
            guac_client_log(client, GUAC_LOG_TRACE, "%s: Received opcode msg on.", __func__);
            break;

        case GUAC_LIB5250_OPCODE_MSGOFF:
            guac_client_log(client, GUAC_LOG_TRACE, "%s: Received opcode msg off.", __func__);
            break;

        default:
            guac_client_log(client, GUAC_LOG_TRACE, "%s: Received unknown opcode.", __func__);

    }

    guac_client_log(client, GUAC_LOG_TRACE, "%s: SNA data length: %d bytes.", __func__, ntohs(packet->len));
    guac_client_log(client, GUAC_LOG_TRACE, "%s: Packet type: 0x%x", __func__, ntohs(packet->type));
    guac_client_log(client, GUAC_LOG_TRACE, "%s: Reserved space: 0x%x", __func__, ntohs(packet->reserved));
    guac_client_log(client, GUAC_LOG_TRACE, "%s: Variable header length: 0x%x.", __func__, packet->varlen);
    guac_client_log(client, GUAC_LOG_TRACE, "%s: SNA Flags: 0x%x", __func__, packet->flags);
    guac_client_log(client, GUAC_LOG_TRACE, "%s: SNA Rerved flags: 0x%x", __func__, packet->reserved_flags);
    guac_client_log(client, GUAC_LOG_TRACE, "%s: SNA Opcode: 0x%x", __func__, packet->opcode);

    guac_client_log(client, GUAC_LOG_TRACE, "%s: WHOLE ENCHILADA: 0x%x", __func__, packet);

    guac_client_log(client, GUAC_LOG_WARNING, "%s: Everything else is unimplemented, now.", __func__);
    return 0;

}