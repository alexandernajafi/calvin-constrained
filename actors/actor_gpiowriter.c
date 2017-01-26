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
#include "actor_gpiowriter.h"
#include "../msgpack_helper.h"
#include "../fifo.h"

result_t actor_gpiowriter_init(actor_t **actor, list_t *attributes)
{
	state_gpiowriter_t *state = NULL;
	char *data = NULL;

	if (platform_mem_alloc((void **)&state, sizeof(state_gpiowriter_t)) != SUCCESS) {
		log_error("Failed to allocate memory");
		return FAIL;
	}
	memset(state, 0, sizeof(state_gpiowriter_t));

	data = (char *)list_get(attributes, "gpio_pin");
	if (data == NULL || decode_uint(data, &state->pin) != SUCCESS) {
		log_error("Failed to get 'gpio_pin'");
		return FAIL;
	}

	state->gpio = platform_create_out_gpio(state->pin);
	if (state->gpio == NULL) {
		log_error("Failed create gpio");
		return FAIL;
	}

	(*actor)->instance_state = (void *)state;

	return SUCCESS;
}

result_t actor_gpiowriter_set_state(actor_t **actor, list_t *attributes)
{
	return actor_gpiowriter_init(actor, attributes);
}

bool actor_gpiowriter_fire(struct actor_t *actor)
{
	state_gpiowriter_t *gpio_state = (state_gpiowriter_t *)actor->instance_state;
	port_t *inport = (port_t *)actor->in_ports->data;
	token_t *in_token = NULL;
	uint32_t in_data = 0;

	if (fifo_tokens_available(&inport->fifo, 1) == 1) {
		in_token = fifo_peek(&inport->fifo);

		if (token_decode_uint(*in_token, &in_data) == SUCCESS) {
			platform_set_gpio(gpio_state->gpio, in_data);
			fifo_commit_read(&inport->fifo);
			return true;
		}

		log_error("Failed to decode token");
		fifo_cancel_commit(&inport->fifo);
	}

	return false;
}

void actor_gpiowriter_free(actor_t *actor)
{
	state_gpiowriter_t *state = (state_gpiowriter_t *)actor->instance_state;

	if (state != NULL) {
		if (state->gpio != NULL)
			platform_uninit_gpio(state->gpio);
		platform_mem_free((void *)state);
	}
}

result_t actor_gpiowriter_get_managed_attributes(actor_t *actor, list_t **attributes)
{
	// TODO: Implement
	return SUCCESS;
}
