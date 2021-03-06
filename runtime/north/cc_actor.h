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
#ifndef ACTOR_H
#define ACTOR_H

#include <stdbool.h>
#include "cc_common.h"
#include "cc_port.h"
#include "../south/platform/cc_platform.h"
#include "../../calvinsys/cc_calvinsys.h"

struct node_t;

typedef enum {
	ACTOR_PENDING,
	ACTOR_ENABLED
} actor_state_t;

typedef struct actor_t {
	char id[UUID_BUFFER_SIZE];
	char name[UUID_BUFFER_SIZE];
	char type[UUID_BUFFER_SIZE];
	char migrate_to[UUID_BUFFER_SIZE]; // id of rt to migrate to if requested
	actor_state_t state;
	void *instance_state;
	list_t *in_ports;
	list_t *out_ports;
	list_t *attributes;
	result_t (*init)(struct actor_t **actor, list_t *attributes);
	result_t (*set_state)(struct actor_t **actor, list_t *attributes);
	bool (*fire)(struct actor_t *actor);
	void (*free_state)(struct actor_t *actor);
	result_t (*get_managed_attributes)(struct actor_t *actor, list_t **attributes);
	void (*will_migrate)(struct actor_t *actor);
	void (*will_end)(struct actor_t *actor);
	void (*did_migrate)(struct actor_t *actor);
	calvinsys_t *calvinsys;
} actor_t;

void actor_set_state(actor_t *actor, actor_state_t state);
actor_t *actor_create(struct node_t *node, char *root);
void actor_free(struct node_t *node, actor_t *actor, bool remove_from_registry);
actor_t *actor_get(struct node_t *node, const char *actor_id, uint32_t actor_id_len);
void actor_port_enabled(actor_t *actor);
void actor_port_disconnected(actor_t *actor);
void actor_disconnect(struct node_t *node, actor_t *actor);
result_t actor_migrate(struct node_t *node, actor_t *actor, char *to_rt_uuid, uint32_t to_rt_uuid_len);
char *actor_serialize_managed_list(list_t *managed_attributes, char **buffer);
char *actor_serialize(const struct node_t *node, const actor_t *actor, char **buffer, bool include_state);

#endif /* ACTOR_H */
