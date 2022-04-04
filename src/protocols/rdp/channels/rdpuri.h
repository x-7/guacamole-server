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

#ifndef GUAC_RDP_CHANNELS_RDPURI_H
#define GUAC_RDP_CHANNELS_RDPURI_H

#include "channels/common-svc.h"

#include <freerdp/freerdp.h>
#include <guacamole/client.h>
#include <guacamole/user.h>
#include <winpr/stream.h>
#include <winpr/wtypes.h>

/**
 * Initializes clipboard support for RDP and handling of the RDPURI channel.
 * If failures occur, messages noting the specifics of those failures will be
 * logged, and the RDP side of URI redirection will not be functional.
 *
 * This MUST be called within the PreConnect callback of the freerdp instance
 * for RDPURI support to be loaded.
 *
 * @param context
 *     The rdpContext associated with the FreeRDP side of the RDP connection.
 */
void guac_rdpuri_load_plugin(rdpContext* context);

#endif

