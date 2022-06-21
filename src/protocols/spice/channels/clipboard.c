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
#include "client.h"
#include "clipboard.h"
#include "common/clipboard.h"
#include "common/iconv.h"
#include "spice.h"
#include "spice-constants.h"
#include "user.h"

#include <guacamole/client.h>
#include <guacamole/stream.h>
#include <guacamole/user.h>

#include <spice-1/spice/vd_agent.h>

int guac_spice_set_clipboard_encoding(guac_client* client,
        const char* name) {

    guac_client_log(client, GUAC_LOG_DEBUG, "Setting clipboard encoding.");

    return 0;

}

int guac_spice_clipboard_handler(guac_user* user, guac_stream* stream,
        char* mimetype) {

    guac_client_log(user->client, GUAC_LOG_DEBUG, "Calling SPICE clipboard handler.");
    guac_spice_client* spice_client = (guac_spice_client*) user->client->data;
    guac_common_clipboard_reset(spice_client->clipboard, mimetype);

    /* Set handlers for clipboard stream */
    stream->blob_handler = guac_spice_clipboard_blob_handler;
    stream->end_handler = guac_spice_clipboard_end_handler;

    return 0;
}

int guac_spice_clipboard_blob_handler(guac_user* user, guac_stream* stream,
        void* data, int length) {

    guac_client_log(user->client, GUAC_LOG_DEBUG, "Calling SPICE clipboard BLOB handler.");

    /* Append new data */
    guac_spice_client* spice_client = (guac_spice_client*) user->client->data;
    guac_common_clipboard_append(spice_client->clipboard, (char*) data, length);

    return 0;
}

int guac_spice_clipboard_end_handler(guac_user* user, guac_stream* stream) {

    guac_client_log(user->client, GUAC_LOG_DEBUG, "Calling SPICE clipboard end handler.");

    guac_spice_client* spice_client = (guac_spice_client*) user->client->data;
    const char* input = spice_client->clipboard->buffer;

    /* Send via VNC only if finished connecting */
    if (spice_client->main_channel != NULL)
        spice_main_channel_clipboard_selection_notify(spice_client->main_channel,
            VD_AGENT_CLIPBOARD_SELECTION_CLIPBOARD,
            VD_AGENT_CLIPBOARD_UTF8_TEXT,
            (const unsigned char*) input,
            spice_client->clipboard->length);

    return 0;
}

void guac_spice_clipboard_selection_handler(SpiceMainChannel* channel,
        guint selection, guint type, gpointer data, guint size,
        guac_client* client) {

    guac_client_log(client, GUAC_LOG_DEBUG, "Notifying client of clipboard data"
            " available from the guest.");

    guac_spice_client* spice_client = (guac_spice_client*) client->data;

    switch (type) {
        case VD_AGENT_CLIPBOARD_UTF8_TEXT:
            guac_common_clipboard_append(spice_client->clipboard, (char *) data, size);
            break;

        default:
            guac_client_log(client, GUAC_LOG_WARNING, "Guacamole currently does"
                " not support clipboard data other than plain text.");
    }

}

void guac_spice_clipboard_selection_grab_handler(SpiceMainChannel* channel,
        guint selection, guint32* types, guint ntypes, guac_client* client) {

    guac_client_log(client, GUAC_LOG_DEBUG, "Notifying client of clipboard grab"
            " in the guest.");
    guac_client_log(client, GUAC_LOG_DEBUG, "Arg: channel: 0x%08x", channel);
    guac_client_log(client, GUAC_LOG_DEBUG, "Arg: selection: 0x%08x", selection);
    guac_client_log(client, GUAC_LOG_DEBUG, "Arg: types: 0x%08x", types);
    guac_client_log(client, GUAC_LOG_DEBUG, "Arg: ntypes: 0x%08x", ntypes);

}

void guac_spice_clipboard_selection_release_handler(SpiceMainChannel* channel,
        guint selection, guac_client* client) {

    guac_client_log(client, GUAC_LOG_DEBUG, "Notifying client of clipboard"
            " release in the guest.");

}

void guac_spice_clipboard_selection_request_handler(SpiceMainChannel* channel,
        guint selection, guint types, guac_client* client) {

    guac_client_log(client, GUAC_LOG_DEBUG, "Requesting clipboard data from"
            " the client.");

    guac_spice_client* spice_client = (guac_spice_client*) client->data;
    const char* input = spice_client->clipboard->buffer;

    spice_main_channel_clipboard_selection_notify(spice_client->main_channel,
            VD_AGENT_CLIPBOARD_SELECTION_CLIPBOARD,
            VD_AGENT_CLIPBOARD_UTF8_TEXT,
            (const unsigned char*) input,
            spice_client->clipboard->length);

}
