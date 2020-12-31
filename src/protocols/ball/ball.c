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

#include "ball.h"

#include <guacamole/client.h>
#include <guacamole/layer.h>
#include <guacamole/protocol.h>
#include <guacamole/socket.h>
#include <guacamole/user.h>

#include <stdlib.h>

const char* TUTORIAL_ARGS[] = { NULL };

int ball_join_handler(guac_user* user, int argc, char** argv) {

    /* Get client associated with user */
    guac_client* client = user->client;
    
    /* Get ball layer from client data */
    ball_client_data* data = (ball_client_data*) client->data;
    guac_layer* ball = data->ball;

    /* Get user-specific socket */
    guac_socket* socket = user->socket;

    /* Send the display size */
    guac_protocol_send_size(socket, GUAC_DEFAULT_LAYER, 1024, 768);

    /* Prepare a curve which covers the entire layer */
    guac_protocol_send_rect(socket, GUAC_DEFAULT_LAYER,
            0, 0, 1024, 768);

    /* Fill curve with solid color */
    guac_protocol_send_cfill(socket,
            GUAC_COMP_OVER, GUAC_DEFAULT_LAYER,
            0x80, 0x80, 0x80, 0xFF);

    /* Set up ball layer */
    guac_protocol_send_size(socket, ball, 128, 128);

    /* Prepare a curve which covers the entire layer */
    guac_protocol_send_rect(socket, ball,
            0, 0, 128, 128);

    /* Fill curve with solid color */
    guac_protocol_send_cfill(socket,
            GUAC_COMP_OVER, ball,
            0x00, 0x80, 0x80, 0xFF);
    
    /* Mark end-of-frame */
    guac_protocol_send_sync(socket, client->last_sent_timestamp);

    /* Flush buffer */
    guac_socket_flush(socket);

    /* User successfully initialized */
    return 0;

}

int ball_free_handler(guac_client* client) {

    ball_client_data* data = (ball_client_data*) client->data;

    /* Free client-level ball layer */
    guac_client_free_layer(client, data->ball);

    /* Free client-specific data */
    free(data);

    /* Data successfully freed */
    return 0;

}


int guac_client_init(guac_client* client) {

    /* Allocate storage for client-specific data */
    ball_client_data* data = malloc(sizeof(ball_client_data));

    /* Set up client data and handlers */
    client->data = data;

    /* Allocate layer at the client level */
    data->ball = guac_client_alloc_layer(client);
    
    /* This example does not implement any arguments */
    client->args = TUTORIAL_ARGS;

    /* Client-level handlers */
    client->join_handler = ball_join_handler;
    client->free_handler = ball_free_handler;

    return 0;

}
