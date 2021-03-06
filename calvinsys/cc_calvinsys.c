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
#include <string.h>
#include "../runtime/north/cc_common.h"
#include "../runtime/south/platform/cc_platform.h"
#include "cc_calvinsys.h"

static result_t calvinsys_add_object_to_handler(calvinsys_obj_t* obj, calvinsys_handler_t* handler)
{
	calvinsys_obj_t* tmp = handler->objects;
	if(tmp == NULL) {
		handler->objects = obj;
		return CC_RESULT_SUCCESS;
	}

	while(tmp->next != NULL)
		tmp = tmp->next;
	tmp->next = obj;

	return CC_RESULT_SUCCESS;
}

void calvinsys_add_handler(calvinsys_t **calvinsys, calvinsys_handler_t *handler)
{
	calvinsys_handler_t *tmp_handler = NULL;

	handler->calvinsys = *calvinsys;

	if ((*calvinsys)->handlers == NULL) {
		(*calvinsys)->handlers = handler;
	} else {
		tmp_handler = (*calvinsys)->handlers;
		while (tmp_handler->next != NULL)
			tmp_handler = tmp_handler->next;
		tmp_handler->next = handler;
	}
}

void calvinsys_delete_handler(calvinsys_handler_t *handler)
{
	calvinsys_obj_t *obj = NULL;

	while (handler->objects != NULL) {
		obj = handler->objects;
		handler->objects = handler->objects->next;
		platform_mem_free((void *)obj);
	}

	platform_mem_free((void *)handler);
}

result_t calvinsys_register_capability(calvinsys_t *calvinsys, const char *name, calvinsys_handler_t *handler, void *state)
{
	calvinsys_capability_t *capability = NULL;

	if (list_get(calvinsys->capabilities, name) != NULL) {
		cc_log_error("Capability '%s' already registered", name);
		return CC_RESULT_FAIL;
	}

	if (platform_mem_alloc((void **)&capability, sizeof(calvinsys_capability_t)) != CC_RESULT_SUCCESS) {
		cc_log_error("Failed to allocate memory");
		return CC_RESULT_FAIL;
	}

	capability->handler = handler;
	capability->state = state;

	if (list_add_n(&calvinsys->capabilities, name, strlen(name), capability, sizeof(calvinsys_capability_t)) == CC_RESULT_SUCCESS) {
		return CC_RESULT_SUCCESS;
	}

	return CC_RESULT_FAIL;
}

void calvinsys_delete_capabiltiy(calvinsys_t *calvinsys, const char *name)
{
	calvinsys_capability_t *capability = NULL;

	capability = (calvinsys_capability_t *)list_get(calvinsys->capabilities, name);
	if (capability != NULL) {
		platform_mem_free(capability->state);
		list_remove(&calvinsys->capabilities, name);
	}
}

calvinsys_obj_t *calvinsys_open(calvinsys_t *calvinsys, const char *name, char *data, size_t size)
{
	calvinsys_capability_t *capability = NULL;

	capability = (calvinsys_capability_t *)list_get(calvinsys->capabilities, name);
	if (capability != NULL) {
		calvinsys_obj_t* result = capability->handler->open(capability->handler, data, size, capability->state, calvinsys->next_id, name);
		if (result != NULL) {
			result->id = calvinsys->next_id++;
			calvinsys_add_object_to_handler(result, capability->handler);
		}
		return result;
	}
	return NULL;
}

void calvinsys_close(calvinsys_obj_t *obj)
{
	if (obj->close != NULL)
		obj->close(obj);
	obj->handler->objects = NULL; // TODO: Handle multiple objects
	platform_mem_free((void *)obj);
}

result_t calvinsys_get_obj_by_id(calvinsys_obj_t** obj, calvinsys_handler_t* handler, uint32_t id)
{
	calvinsys_obj_t* tmp = handler->objects;

	while(tmp != NULL) {
		if (tmp->id == id){
			*obj = tmp;
			return CC_RESULT_SUCCESS;
		}
		tmp = tmp->next;
	}
	return CC_RESULT_FAIL;
}