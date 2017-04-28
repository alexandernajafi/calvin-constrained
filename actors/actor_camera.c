/*
 * Copyright (c) 2016 Ericsson AB
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdbool.h>
#include <common.h>
#include <actor.h>
#include <calvinsys.h>
#include <string.h>
#include "actor_camera.h"

result_t actor_camera_init(actor_t **actor, list_t *attributes)
{
	log("camera actor init");
	calvinsys_t* camera = (calvinsys_t*) list_get((*actor)->calvinsys, "calvinsys.media.camerahandler");
	if (camera == NULL) {
		log_error("calvinsys.media.camerahandler not supported");
		return FAIL;
	}
	state_camera_t* state;
	if (platform_mem_alloc((void**)&state, sizeof(state_camera_t)) != SUCCESS) {
		platform_mem_free(camera);
		log_error("Could not allocate memory for button state");
		return FAIL;
	}
	camera->init(camera);
	state->camera = camera;
	(*actor)->instance_state = (void*) state;
	return SUCCESS;
}

bool actor_camera_fire(struct actor_t *actor)
{
	if (actor->instance_state != NULL) {
		// Make sure that we can write
		calvinsys_t *camera = ((state_camera_t *) actor->instance_state)->camera;
		port_t *inport = (port_t *)actor->in_ports->data, *outport = (port_t *)actor->out_ports->data;
		token_t* in_token = NULL;
		if(camera != NULL && fifo_tokens_available(&inport->fifo, 1)) {
			// Received trigger, tell camera sys to take a picture!
			in_token = fifo_peek(&inport->fifo);
			if (camera->write(camera, "trigger", NULL, 0) != SUCCESS) {
				log_error("Could not twrite trigger command to camera..");
				return false;
			} else {
				log("Camera wrote trigger command!");
				fifo_commit_read(&inport->fifo);
			}
		} else if (camera != NULL && camera->new_data) {
			// Process data from camera sys
			log("data from camera!");
			if (strncmp(camera->command, "image", 5) == 0) {
				// Receiveid new image
				log("data was image of size %zu", camera->data_size);
				if (fifo_slots_available(&outport->fifo, 1)) {
					// Got new image, write on outport
					token_t out_token;
					token_set_str(&out_token, camera->data, camera->data_size);
					if (fifo_write(&outport->fifo, out_token.value, out_token.size) != SUCCESS) {
						log_error("send camera data token failed");
					} else {
						camera->new_data = 0;
						camera->data_size = 0;
						platform_mem_free(camera->command);
						platform_mem_free(camera->data);
						return true;
					}
				}
			}
		}
	}
	return false;
}

result_t actor_camera_set_state(actor_t **actor, list_t *attributes)
{
	log("camera set state");
	actor_camera_init(actor, attributes);
}

void actor_camera_will_end(actor_t* actor)
{
	log("camera end");
	if(actor->instance_state != NULL) {
		calvinsys_t *camera = ((state_camera_t *) actor->instance_state)->camera;
		camera->release(camera);
	}
}

void actor_camera_free(actor_t *actor)
{
	log("actor free");
	if(actor->instance_state != NULL) {
		calvinsys_t *camera = ((state_camera_t *) actor->instance_state)->camera;
		release_calvinsys(&camera);
		platform_mem_free(actor->instance_state);
	}
}