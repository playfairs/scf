#ifndef SCF_PROVIDER_H
#define SCF_PROVIDER_H

#include <scf/scf.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCF_PROVIDER_NAME_MAX 64

typedef enum {
	SCF_PROVIDER_HASH = 1,
	SCF_PROVIDER_CIPHER = 2,
	SCF_PROVIDER_KDF = 3,
	SCF_PROVIDER_FORMAT = 4
} scf_provider_type;

typedef enum {
	SCF_PROVIDER_CAP_CONTEXT = 1u << 0,
	SCF_PROVIDER_CAP_STREAMING = 1u << 1,
	SCF_PROVIDER_CAP_HARDWARE = 1u << 2
} scf_provider_capability;

typedef scf_status (*scf_provider_context_create_fn)(scf_const_buffer parameters, void **state);
typedef void (*scf_provider_context_destroy_fn)(void *state);

typedef struct {
	scf_provider_type type;
	uint32_t identifier;
	const char *name;
	uint32_t capabilities;
	scf_provider_context_create_fn context_create;
	scf_provider_context_destroy_fn context_destroy;
} scf_provider_descriptor;

typedef struct {
	scf_provider_type type;
	uint32_t identifier;
	char name[SCF_PROVIDER_NAME_MAX];
	uint32_t capabilities;
} scf_provider_info;

typedef struct scf_provider_registry scf_provider_registry;
typedef struct scf_provider_context scf_provider_context;

SCF_API scf_status scf_provider_registry_create(scf_provider_registry **registry);
SCF_API scf_status scf_provider_register(scf_provider_registry *registry, const scf_provider_descriptor *descriptor);
SCF_API scf_status scf_provider_unregister(scf_provider_registry *registry, scf_provider_type type, uint32_t identifier);
SCF_API scf_status scf_provider_lookup(const scf_provider_registry *registry, scf_provider_type type, uint32_t identifier, scf_provider_info *info);
SCF_API scf_status scf_provider_context_create(scf_provider_registry *registry, scf_provider_type type, uint32_t identifier, scf_const_buffer parameters, scf_provider_context **context);
SCF_API scf_status scf_provider_context_info(const scf_provider_context *context, scf_provider_info *info);
SCF_API void scf_provider_context_destroy(scf_provider_context *context);
SCF_API scf_status scf_provider_registry_destroy(scf_provider_registry *registry);

#ifdef __cplusplus
}
#endif

#endif