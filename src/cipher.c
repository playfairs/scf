#include "internal.h"

struct scf_cipher_context {
	const scf_cipher_provider *provider;
	void *state;
	scf_byte *key;
	scf_byte *nonce;
};

scf_status scf_cipher_context_create(const scf_cipher_provider *provider, scf_const_buffer key, scf_const_buffer nonce, scf_cipher_context **context)
{
	if (provider == NULL || context == NULL || provider->init == NULL || provider->encrypt == NULL || provider->decrypt == NULL || provider->reset == NULL || provider->destroy == NULL || !scf_internal_buffer_valid(key) || !scf_internal_buffer_valid(nonce)) {
		return SCF_STATUS_INVALID_ARGUMENT;
	}
	if (key.size != provider->key_size || nonce.size != provider->nonce_size) {
		return SCF_STATUS_INVALID_ARGUMENT;
	}

	*context = scf_internal_alloc(sizeof(**context));
	if (*context == NULL) {
		return SCF_STATUS_ALLOCATION_FAILED;
	}
	(*context)->provider = provider;
	(*context)->state = NULL;
	(*context)->key = NULL;
	(*context)->nonce = NULL;
	if (key.size > 0) {
		(*context)->key = scf_internal_alloc(key.size);
	}
	if (nonce.size > 0) {
		(*context)->nonce = scf_internal_alloc(nonce.size);
	}
	if ((key.size > 0 && (*context)->key == NULL) || (nonce.size > 0 && (*context)->nonce == NULL)) {
		scf_cipher_context_destroy(*context);
		*context = NULL;
		return SCF_STATUS_ALLOCATION_FAILED;
	}
	scf_internal_copy((*context)->key, key.data, key.size);
	scf_internal_copy((*context)->nonce, nonce.data, nonce.size);
	if (provider->init(&(*context)->state, (scf_const_buffer){(*context)->key, key.size}, (scf_const_buffer){(*context)->nonce, nonce.size}) != SCF_STATUS_SUCCESS) {
		scf_cipher_context_destroy(*context);
		*context = NULL;
		return SCF_STATUS_INVALID_STATE;
	}
	return SCF_STATUS_SUCCESS;
}

static scf_status scf_cipher_crypt(scf_cipher_context *context, scf_const_buffer input, scf_buffer output, scf_cipher_crypt_fn operation)
{
	if (context == NULL || !scf_internal_buffer_valid(input) || !scf_internal_output_valid(output) || operation == NULL) {
		return SCF_STATUS_INVALID_ARGUMENT;
	}
	if (output.size < input.size) {
		return SCF_STATUS_BUFFER_TOO_SMALL;
	}
	return operation(context->state, input, output);
}

scf_status scf_cipher_encrypt(scf_cipher_context *context, scf_const_buffer input, scf_buffer output)
{
	return scf_cipher_crypt(context, input, output, context == NULL ? NULL : context->provider->encrypt);
}

scf_status scf_cipher_decrypt(scf_cipher_context *context, scf_const_buffer input, scf_buffer output)
{
	return scf_cipher_crypt(context, input, output, context == NULL ? NULL : context->provider->decrypt);
}

scf_status scf_cipher_reset(scf_cipher_context *context)
{
	if (context == NULL) {
		return SCF_STATUS_INVALID_ARGUMENT;
	}
	return context->provider->reset(context->state);
}

void scf_cipher_context_destroy(scf_cipher_context *context)
{
	if (context != NULL) {
		context->provider->destroy(context->state);
		scf_internal_clear(context->key, context->provider->key_size);
		scf_internal_clear(context->nonce, context->provider->nonce_size);
		scf_internal_free(context->key);
		scf_internal_free(context->nonce);
		scf_internal_clear(context, sizeof(*context));
		scf_internal_free(context);
	}
}
