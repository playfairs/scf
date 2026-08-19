#include "internal.h"

struct scf_key {
	scf_secure_allocation *material;
	scf_key_state state;
};

scf_status scf_key_create(scf_const_buffer material, scf_key **key)
{
	if (key == NULL || !scf_internal_buffer_valid(material) || material.size == 0) {
		return SCF_STATUS_INVALID_ARGUMENT;
	}
	*key = scf_internal_alloc(sizeof(**key));
	if (*key == NULL) {
		return SCF_STATUS_ALLOCATION_FAILED;
	}
	scf_status status = scf_secure_allocate(material.size, 0, &(*key)->material);
	if (status != SCF_STATUS_SUCCESS) {
		scf_internal_free(*key);
		*key = NULL;
		return status;
	}
	scf_buffer destination;
	scf_secure_data((*key)->material, &destination);
	scf_internal_copy(destination.data, material.data, material.size);
	(*key)->state = SCF_KEY_INITIALIZED;
	return SCF_STATUS_SUCCESS;
}

scf_status scf_key_copy(const scf_key *source, scf_key **copy)
{
	if (source == NULL || copy == NULL || source->state != SCF_KEY_INITIALIZED) {
		return SCF_STATUS_INVALID_ARGUMENT;
	}
	scf_const_buffer material;
	if (scf_key_data(source, &material) != SCF_STATUS_SUCCESS) {
		return SCF_STATUS_INVALID_STATE;
	}
	return scf_key_create(material, copy);
}

scf_status scf_key_data(const scf_key *key, scf_const_buffer *material)
{
	if (key == NULL || material == NULL || key->state != SCF_KEY_INITIALIZED) {
		return SCF_STATUS_INVALID_STATE;
	}
	scf_buffer data;
	if (scf_secure_data(key->material, &data) != SCF_STATUS_SUCCESS) {
		return SCF_STATUS_INVALID_STATE;
	}
	material->data = data.data;
	material->size = data.size;
	return SCF_STATUS_SUCCESS;
}

scf_size scf_key_size(const scf_key *key)
{
	scf_size size;
	return key == NULL || scf_secure_size(key->material, &size) != SCF_STATUS_SUCCESS ? 0 : size;
}

scf_key_state scf_key_state_get(const scf_key *key)
{
	return key == NULL ? 0 : key->state;
}

scf_status scf_key_clear(scf_key *key)
{
	if (key == NULL) {
		return SCF_STATUS_INVALID_ARGUMENT;
	}
	scf_secure_clear(key->material);
	key->state = SCF_KEY_CLEARED;
	return SCF_STATUS_SUCCESS;
}

void scf_key_destroy(scf_key *key)
{
	if (key != NULL) {
		scf_secure_destroy(key->material);
		scf_internal_clear(key, sizeof(*key));
		scf_internal_free(key);
	}
}
