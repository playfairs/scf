#ifndef SCF_CIPHER_H
#define SCF_CIPHER_H

#include <scf/scf.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct scf_cipher_context scf_cipher_context;

typedef scf_status (*scf_cipher_init_fn)(void **state, scf_const_buffer key, scf_const_buffer nonce);
typedef scf_status (*scf_cipher_crypt_fn)(void *state, scf_const_buffer input, scf_buffer output);
typedef scf_status (*scf_cipher_reset_fn)(void *state);
typedef void (*scf_cipher_destroy_fn)(void *state);

typedef struct {
	const char *name;
	scf_size key_size;
	scf_size nonce_size;
	scf_cipher_init_fn init;
	scf_cipher_crypt_fn encrypt;
	scf_cipher_crypt_fn decrypt;
	scf_cipher_reset_fn reset;
	scf_cipher_destroy_fn destroy;
} scf_cipher_provider;

SCF_API scf_status scf_cipher_context_create(const scf_cipher_provider *provider, scf_const_buffer key, scf_const_buffer nonce, scf_cipher_context **context);
SCF_API scf_status scf_cipher_encrypt(scf_cipher_context *context, scf_const_buffer input, scf_buffer output);
SCF_API scf_status scf_cipher_decrypt(scf_cipher_context *context, scf_const_buffer input, scf_buffer output);
SCF_API scf_status scf_cipher_reset(scf_cipher_context *context);
SCF_API void scf_cipher_context_destroy(scf_cipher_context *context);

#ifdef __cplusplus
}
#endif

#endif
