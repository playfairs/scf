#ifndef SCF_KAT_H
#define SCF_KAT_H

#include <scf/memory.h>
#include <scf/provider.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SCF_KAT_NAME_MAX 64

    typedef struct
    {
        scf_const_buffer input;
        scf_const_buffer expected;
        scf_const_buffer key;
        scf_const_buffer nonce;
        scf_const_buffer salt;
        scf_const_buffer auxiliary;
    } scf_kat_vector;

    typedef scf_status (*scf_kat_execute_fn)(
        const scf_kat_vector *vector);

    typedef struct
    {
        scf_provider_type provider_type;
        uint32_t provider_identifier;
        uint32_t test_identifier;
        const char *name;
        scf_kat_vector vector;
        scf_kat_execute_fn execute;
    } scf_kat_descriptor;

    typedef struct
    {
        scf_status status;
        scf_provider_type provider_type;
        uint32_t provider_identifier;
        uint32_t test_identifier;
        char provider_name[SCF_PROVIDER_NAME_MAX];
        char test_name[SCF_KAT_NAME_MAX];
    } scf_kat_result;

    typedef struct scf_kat_registry scf_kat_registry;

    SCF_API scf_status scf_kat_registry_create(
        scf_provider_registry *providers,
        scf_kat_registry **registry);
    SCF_API scf_status
    scf_kat_register(scf_kat_registry *registry,
                     const scf_kat_descriptor *descriptor);
    SCF_API scf_status
    scf_kat_unregister(scf_kat_registry *registry,
                       scf_provider_type provider_type,
                       uint32_t provider_identifier,
                       uint32_t test_identifier);
    SCF_API scf_status
    scf_kat_run(scf_kat_registry *registry,
                scf_provider_type provider_type,
                uint32_t provider_identifier,
                uint32_t test_identifier,
                scf_kat_result *result);
    SCF_API scf_status
    scf_kat_run_provider(scf_kat_registry *registry,
                         scf_provider_type provider_type,
                         uint32_t provider_identifier,
                         scf_kat_result *result,
                         scf_size *executed);
    SCF_API scf_status
    scf_kat_run_all(scf_kat_registry *registry,
                    scf_kat_result *result,
                    scf_size *executed);
    SCF_API scf_status
    scf_kat_registry_destroy(scf_kat_registry *registry);

#ifdef __cplusplus
}
#endif

#endif