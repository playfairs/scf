#include "internal.h"

struct scf_key {
	scf_byte *data;
	scf_size size;
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
	(*key)->data = scf_internal_alloc(material.size);
	if ((*key)->data == NULL) {
		scf_internal_free(*key);
		*key = NULL;
		return SCF_STATUS_ALLOCATION_FAILED;
	}
	scf_internal_copy((*key)->data, material.data, material.size);
	(*key)->size = material.size;
	(*key)->state = SCF_KEY_INITIALIZED;
	return SCF_STATUS_SUCCESS;
}

scf_status scf_key_copy(const scf_key *source, scf_key **copy)
{
	if (source == NULL || copy == NULL || source->state != SCF_KEY_INITIALIZED) {
		return SCF_STATUS_INVALID_ARGUMENT;
	}
	return scf_key_create((scf_const_buffer){source->data, source->size}, copy);
}

scf_status scf_key_data(const scf_key *key, scf_const_buffer *material)
{
	if (key == NULL || material == NULL || key->state != SCF_KEY_INITIALIZED) {
		return SCF_STATUS_INVALID_STATE;
	}
	material->data = key->data;
	material->size = key->size;
	return SCF_STATUS_SUCCESS;
}

scf_size scf_key_size(const scf_key *key)
{
	return key == NULL ? 0 : key->size;
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
	scf_internal_clear(key->data, key->size);
	key->state = SCF_KEY_CLEARED;
	return SCF_STATUS_SUCCESS;
}

void scf_key_destroy(scf_key *key)
{
	if (key != NULL) {
		scf_internal_clear(key->data, key->size);
		scf_internal_free(key->data);
		scf_internal_clear(key, sizeof(*key));
		scf_internal_free(key);
	}
}
