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

#ifndef GUAC_SPICE_CLIPBOARD_H
#define GUAC_SPICE_CLIPBOARD_H

#include "config.h"

#include <guacamole/client.h>
#include <guacamole/user.h>

#include <spice-client-glib-2.0/spice-client.h>

/**
 * Sets the encoding of clipboard data exchanged with the SPICE server to the
 * encoding having the given name. If the name is NULL, or is invalid, the
 * standard ISO8859-1 encoding will be used.
 *
 * @param client
 *     The client to set the clipboard encoding of.
 *
 * @param name
 *     The name of the encoding to use for all clipboard data. Valid values
 *     are: "ISO8859-1", "UTF-8", "UTF-16", "CP1252", or NULL.
 *
 * @return
 *     Zero if the chosen encoding is standard for SPICE, or non-zero if the
 *     SPICE standard is being violated.
 */
int guac_spice_set_clipboard_encoding(guac_client* client,
        const char* name);

/**
 * Handler for inbound clipboard data from Guacamole users.
 */
guac_user_clipboard_handler guac_spice_clipboard_handler;

/**
 * Handler for stream data related to clipboard.
 */
guac_user_blob_handler guac_spice_clipboard_blob_handler;

/**
 * Handler for end-of-stream related to clipboard.
 */
guac_user_end_handler guac_spice_clipboard_end_handler;

/**
 * A handler that will be registered with the SPICE client to handle clipboard
 * data sent from the SPICE server to the client.
 * 
 * @param channel
 *     The main SPICE channel on which this event was fired.
 * 
 * @param selection
 *     The clipboard on which the selection occurred.
 * 
 * @param type
 *     The type of the data that is on the clipboard.
 * 
 * @param data
 *     A pointer to the location containing the data that is on the clipboard.
 * 
 * @param size
 *     The amount of data in bytes.
 * 
 * @param client
 *     The guac_client associated with this event handler, passed when the
 *     handler was registered.
 */
void guac_spice_clipboard_selection_handler(SpiceMainChannel channel,
        guint selection, guint type, gpointer data, guint size,
        guac_client* client);

/**
 * A handler that will be registered with the SPICE client to handle clipboard
 * events where the guest (vdagent) within the SPICE server notifies the client
 * that data is available on the clipboard.
 * 
 * @param channel
 *     The main SpiceChannel on which this event is fired.
 * 
 * @param selection
 *     The SPICE clipboard from which the event is fired.
 * 
 * @param types
 *     The type of data being sent by the agent.
 * 
 * @param ntypes
 *     The number of data types represented.
 * 
 * @param client
 *     The guac_client that was passed in when the callback was registered.
 */
void guac_spice_clipboard_selection_grab_handler(SpiceMainChannel channel,
        guint selection, guint32* types, guint ntypes, guint other, guac_client* client);

/**
 * A handler that will be called by the SPICE client when the SPICE server
 * is done with the clipboard and releases control of it. 
 * 
 * @param chennl
 *     The main SPICE channel on which this event is fired.
 * 
 * @param selection
 *     The SPICE server clipboard releasing control.
 * 
 * @param client
 *     The guac_client that was registered with the callback.
 */
void guac_spice_clipboard_selection_release_handler(SpiceMainChannel channel,
        guint selection, guint other, guac_client* client);

/**
 * A handler that will be called by the SPICE client when the SPICE server
 * would like to check and receive the contents of the client's clipboard.
 * 
 * @param channel
 *     The main SPICE channel on which this event is fired.
 * 
 * @param selection
 *     The SPICE server clipboard that is requesting data.
 * 
 * @param types
 *     The type of data to be sent to the SPICE server.
 * 
 * @param client
 *     The guac_client object that was registered with the callback.
 */
void guac_spice_clipboard_selection_request_handler(SpiceMainChannel channel,
        guint selection, guint types, guac_client* client);

#endif /* GUAC_SPICE_CLIPBOARD_H */

