#include <stdlib.h>
#include <string.h>

#include <scf/provider.h>

static int created;
static int destroyed;

static scf_status provider_create(scf_const_buffer parameters, void **state)
{
    if (parameters.size != 3 || parameters.data == NULL) {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    *state = malloc(1);
    if (*state == NULL) {
        return SCF_STATUS_ALLOCATION_FAILED;
    }
    created++;
    return SCF_STATUS_SUCCESS;
}

static void provider_destroy(void *state)
{
    free(state);
    destroyed++;
}

int scf_unit_provider(void)
{
    const scf_byte parameters[] = {1, 2, 3};
    const scf_provider_descriptor descriptor = {SCF_PROVIDER_HASH, 1, "dummy", SCF_PROVIDER_CAP_CONTEXT, provider_create, provider_destroy};
    const scf_provider_descriptor invalid = {SCF_PROVIDER_HASH, 2, "invalid", 0, provider_create, provider_destroy};
    const scf_provider_descriptor missing_name = {SCF_PROVIDER_HASH, 3, NULL, SCF_PROVIDER_CAP_CONTEXT, provider_create, provider_destroy};
    const scf_provider_descriptor long_name = {SCF_PROVIDER_HASH, 4, "1234567890123456789012345678901234567890123456789012345678901234", SCF_PROVIDER_CAP_CONTEXT, provider_create, provider_destroy};
    scf_provider_registry *registry = NULL;
    scf_provider_context *context = NULL;
    scf_provider_info info;

    created = 0;
    destroyed = 0;
    if (scf_provider_registry_create(&registry) != SCF_STATUS_SUCCESS || scf_provider_register(registry, &descriptor) != SCF_STATUS_SUCCESS || scf_provider_register(registry, &descriptor) != SCF_STATUS_DUPLICATE || scf_provider_register(registry, &invalid) != SCF_STATUS_INVALID_ARGUMENT || scf_provider_register(registry, &missing_name) != SCF_STATUS_INVALID_ARGUMENT || scf_provider_register(registry, &long_name) != SCF_STATUS_NAME_TOO_LONG || scf_provider_lookup(registry, SCF_PROVIDER_HASH, 1, &info) != SCF_STATUS_SUCCESS || strcmp(info.name, "dummy") != 0 || info.capabilities != SCF_PROVIDER_CAP_CONTEXT || scf_provider_lookup(registry, SCF_PROVIDER_HASH, 9, &info) != SCF_STATUS_NOT_FOUND || scf_provider_context_create(registry, SCF_PROVIDER_HASH, 9, (scf_const_buffer){parameters, sizeof(parameters)}, &context) != SCF_STATUS_NOT_FOUND || scf_provider_context_create(registry, SCF_PROVIDER_HASH, 1, (scf_const_buffer){parameters, sizeof(parameters)}, &context) != SCF_STATUS_SUCCESS || created != 1 || scf_provider_context_info(context, &info) != SCF_STATUS_SUCCESS || scf_provider_unregister(registry, SCF_PROVIDER_HASH, 1) != SCF_STATUS_SUCCESS || scf_provider_lookup(registry, SCF_PROVIDER_HASH, 1, &info) != SCF_STATUS_NOT_FOUND || scf_provider_registry_destroy(registry) != SCF_STATUS_BUSY) {
        scf_provider_context_destroy(context);
        if (registry != NULL) {
            scf_provider_unregister(registry, SCF_PROVIDER_HASH, 1);
            scf_provider_registry_destroy(registry);
        }
        return 1;
    }
    scf_provider_context_destroy(context);
    if (destroyed != 1 || scf_provider_registry_destroy(registry) != SCF_STATUS_SUCCESS) {
        return 1;
    }
    return 0;
}