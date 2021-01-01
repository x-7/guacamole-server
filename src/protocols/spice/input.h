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


#ifndef GUAC_SPICE_INPUT_H
#define GUAC_SPICE_INPUT_H

#include "config.h"

#include <guacamole/user.h>

/**
 * Handler for Guacamole user mouse events.
 */
guac_user_mouse_handler guac_spice_user_mouse_handler;

/**
 * Handler for Guacamole user key events.
 */
guac_user_key_handler guac_spice_user_key_handler;

/**
 * A callback that is invoked when the SPICE server updates the mouse mode.
 * 
 * @param channel
 *     The channel on which the update occurred.
 * 
 * @param client
 *     The guac_client instance associated with this session.
 */
void guac_spice_mouse_mode_update(SpiceChannel* channel, guac_client* client);

#endif /* GUAC_SPICE_INPUT_H */

