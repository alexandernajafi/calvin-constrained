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
#include <stdlib.h>
#include <string.h>
#include "cc_actor_temperature.h"
#include "../runtime/north/cc_fifo.h"
#include "../runtime/north/cc_token.h"
#include "../runtime/north/cc_port.h"
#include "../runtime/north/cc_common.h"

result_t actor_temperature_init(actor_t **actor, list_t *attributes)
{
	calvinsys_obj_t *obj = NULL;
	state_temperature_t *state = NULL;

	obj = calvinsys_open((*actor)->calvinsys, "calvinsys.sensors.environmental", NULL, 0);
	if (obj == NULL) {
		log_error("Failed to open 'calvinsys.sensors.environmental'");
		return CC_RESULT_FAIL;
	}

	if (platform_mem_alloc((void **)&state, sizeof(state_temperature_t)) != CC_RESULT_SUCCESS) {
		log_error("Failed to allocate memory");
		obj->close(obj);
		return CC_RESULT_FAIL;
	}

	state->obj = obj;
	(*actor)->instance_state = (void *)state;

	return CC_RESULT_SUCCESS;
}

result_t actor_temperature_set_state(actor_t **actor, list_t *attributes)
{
	return actor_temperature_init(actor, attributes);
}

bool actor_temperature_fire(struct actor_t *actor)
{
	port_t *inport = (port_t *)actor->in_ports->data, *outport = (port_t *)actor->out_ports->data;
	calvinsys_obj_t *obj = ((state_temperature_t *)actor->instance_state)->obj;
	char *data = NULL;
	size_t size = 0;

	if (obj->can_read(obj) && fifo_tokens_available(&inport->fifo, 1) && fifo_slots_available(&outport->fifo, 1)) {
		if (obj->read(obj, &data, &size) == CC_RESULT_SUCCESS) {
			fifo_peek(&inport->fifo);
			if (fifo_write(&outport->fifo, data, size) == CC_RESULT_SUCCESS) {
				fifo_commit_read(&inport->fifo);
				return true;
			}
			fifo_cancel_commit(&inport->fifo);
			platform_mem_free((void *)data);
		} else
			log_error("Failed to read temperature");
	}

	return false;
}

void actor_temperature_free(actor_t *actor)
{
	state_temperature_t *state = (state_temperature_t *)actor->instance_state;

	if (state != NULL) {
		if (state->obj != NULL)
			calvinsys_close(state->obj);
		platform_mem_free((void *)state);
	}
}