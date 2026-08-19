#ifndef SCF_KDF_H
#define SCF_KDF_H

#include <scf/scf.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct scf_kdf_context scf_kdf_context;

typedef scf_status (*scf_kdf_init_fn)(void **state);
typedef scf_status (*scf_kdf_derive_fn)(void *state, scf_const_buffer password, scf_const_buffer salt, scf_buffer output);
typedef scf_status (*scf_kdf_reset_fn)(void *state);
typedef void (*scf_kdf_destroy_fn)(void *state);

typedef struct {
	const char *name;
	scf_size minimum_salt_size;
	scf_size maximum_output_size;
	scf_kdf_init_fn init;
	scf_kdf_derive_fn derive;
	scf_kdf_reset_fn reset;
	scf_kdf_destroy_fn destroy;
} scf_kdf_provider;

SCF_API scf_status scf_kdf_context_create(const scf_kdf_provider *provider, scf_kdf_context **context);
SCF_API scf_status scf_kdf_derive(scf_kdf_context *context, scf_const_buffer password, scf_const_buffer salt, scf_buffer output);
SCF_API scf_status scf_kdf_reset(scf_kdf_context *context);
SCF_API void scf_kdf_context_destroy(scf_kdf_context *context);

#ifdef __cplusplus
}
#endif

#endif
