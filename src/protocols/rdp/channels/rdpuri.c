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

#include "channels/rdpuri.h"
#include "client.h"
#include "config.h"
#include "plugins/channels.h"
#include "rdp.h"
#include "unicode.h"

#include <freerdp/client/cliprdr.h>
#include <freerdp/event.h>
#include <freerdp/freerdp.h>
#include <guacamole/client.h>
#include <guacamole/stream.h>
#include <guacamole/user.h>
#include <winpr/wtsapi.h>
#include <winpr/wtypes.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/**
 * Callback which associates handlers specific to Guacamole with the
 * CliprdrClientContext instance allocated by FreeRDP to deal with received
 * CLIPRDR (clipboard redirection) messages.
 *
 * This function is called whenever a channel connects via the PubSub event
 * system within FreeRDP, but only has any effect if the connected channel is
 * the CLIPRDR channel. This specific callback is registered with the PubSub
 * system of the relevant rdpContext when guac_rdp_clipboard_load_plugin() is
 * called.
 *
 * @param context
 *     The rdpContext associated with the active RDP session.
 *
 * @param e
 *     Event-specific arguments, mainly the name of the channel, and a
 *     reference to the associated plugin loaded for that channel by FreeRDP.
 */
static void guac_rdpuri_channel_connected(guac_rdp_common_svc* svc) {

    guac_client_log(svc->client, GUAC_LOG_DEBUG, "RDPURI (URI redirection) "
            "channel connected.");

}

/**
 * Callback which disassociates Guacamole from the CliprdrClientContext
 * instance that was originally allocated by FreeRDP and is about to be
 * deallocated.
 *
 * This function is called whenever a channel disconnects via the PubSub event
 * system within FreeRDP, but only has any effect if the disconnected channel
 * is the CLIPRDR channel. This specific callback is registered with the PubSub
 * system of the relevant rdpContext when guac_rdp_clipboard_load_plugin() is
 * called.
 *
 * @param context
 *     The rdpContext associated with the active RDP session.
 *
 * @param e
 *     Event-specific arguments, mainly the name of the channel, and a
 *     reference to the associated plugin loaded for that channel by FreeRDP.
 */
static void guac_rdpuri_channel_disconnected(guac_rdp_common_svc* svc) {

    guac_client_log(svc->client, GUAC_LOG_DEBUG, "RDPURI (URI redirection) "
            "channel disconnected.");

}

static void guac_rdpuri_channel_receive(guac_rdp_common_svc* svc, wStream* input_stream) {
    guac_client_log(svc->client, GUAC_LOG_DEBUG, "RDPURI (URI redirection) "
            "channel received data.");

    // int len = Stream_Length(input_stream);
    int len = Stream_Length(input_stream);

    guac_client_log(svc->client, GUAC_LOG_DEBUG, "Received URI %d in length.", len);

    if (len < 1) {
        guac_client_log(svc->client, GUAC_LOG_WARNING, "Received URI data is too "
            "small.");
        return;
    }

    /* Convert input stream to UTF8 and send it to the client. */
    char uri[len/2];
    guac_rdp_utf16_to_utf8(Stream_Pointer(input_stream), len/2 - 1, uri, sizeof(uri));
    guac_client_log(svc->client, GUAC_LOG_DEBUG, "Received URI from server: %s", uri);
    guac_client_owner_send_uri(svc->client, uri);

}

void guac_rdpuri_load_plugin(rdpContext* context) {

    guac_client* client = ((rdp_freerdp_context*) context)->client;

    /* Attempt to load FreeRDP support for the CLIPRDR channel */
    if (guac_rdp_common_svc_load_plugin(context, "rdpuri", 0, 
                guac_rdpuri_channel_connected, guac_rdpuri_channel_receive,
                guac_rdpuri_channel_disconnected)) {
        guac_client_log(client, GUAC_LOG_WARNING,
                "Support for the RDPURI channel (URI redirection) "
                "could not be loaded. This support normally takes the form of "
                "a plugin which is built into FreeRDP. Lacking this support, "
                "URI redirection will not work.");
        return;
    }

    guac_client_log(client, GUAC_LOG_DEBUG, "Support for RDPURI "
            "(URI redirection) registered. Awaiting channel "
            "connection.");

}