#include <string.h>

#include <scf/kat.h>

static int executed;

static scf_status
kat_provider_create(scf_const_buffer parameters,
                    void **state)
{
    (void)parameters;
    *state = NULL;
    return SCF_STATUS_SUCCESS;
}

static void kat_provider_destroy(void *state)
{
    (void)state;
}

static scf_status kat_pass(const scf_kat_vector *vector)
{
    executed++;
    if (vector->input.size != vector->expected.size
        || memcmp(vector->input.data,
                  vector->expected.data,
                  vector->input.size)
               != 0)
    {
        return SCF_STATUS_TEST_FAILED;
    }
    return SCF_STATUS_SUCCESS;
}

static scf_status kat_fail(const scf_kat_vector *vector)
{
    (void)vector;
    executed++;
    return SCF_STATUS_TEST_FAILED;
}

static scf_status
kat_unsupported(const scf_kat_vector *vector)
{
    (void)vector;
    executed++;
    return SCF_STATUS_UNSUPPORTED;
}

int scf_unit_kat(void)
{
    const scf_provider_descriptor provider = {
        SCF_PROVIDER_HASH,
        77,
        "kat-dummy",
        SCF_PROVIDER_CAP_CONTEXT,
        kat_provider_create,
        kat_provider_destroy};
    const scf_provider_descriptor empty_provider = {
        SCF_PROVIDER_HASH,
        78,
        "kat-empty",
        SCF_PROVIDER_CAP_CONTEXT,
        kat_provider_create,
        kat_provider_destroy};
    const scf_byte expected[] = {4, 5, 6};
    const scf_byte input[] = {4, 5, 6};
    const scf_kat_descriptor pass = {
        SCF_PROVIDER_HASH,
        77,
        1,
        "pass",
        {{input, sizeof(input)},
         {expected, sizeof(expected)},
         {NULL, 0},
         {NULL, 0},
         {NULL, 0},
         {NULL, 0}},
        kat_pass};
    const scf_kat_descriptor fail = {
        SCF_PROVIDER_HASH,
        77,
        2,
        "fail",
        {{input, sizeof(input)},
         {expected, sizeof(expected)},
         {NULL, 0},
         {NULL, 0},
         {NULL, 0},
         {NULL, 0}},
        kat_fail};
    const scf_kat_descriptor unsupported = {
        SCF_PROVIDER_HASH,
        77,
        3,
        "unsupported",
        {{NULL, 0},
         {NULL, 0},
         {NULL, 0},
         {NULL, 0},
         {NULL, 0},
         {NULL, 0}},
        kat_unsupported};
    const scf_kat_descriptor invalid = {SCF_PROVIDER_HASH,
                                        77,
                                        4,
                                        NULL,
                                        {{NULL, 0},
                                         {NULL, 0},
                                         {NULL, 0},
                                         {NULL, 0},
                                         {NULL, 0},
                                         {NULL, 0}},
                                        kat_pass};
    const scf_kat_descriptor missing = {SCF_PROVIDER_HASH,
                                        999,
                                        5,
                                        "missing",
                                        {{NULL, 0},
                                         {NULL, 0},
                                         {NULL, 0},
                                         {NULL, 0},
                                         {NULL, 0},
                                         {NULL, 0}},
                                        kat_pass};
    scf_provider_registry *providers = NULL;
    scf_kat_registry *registry = NULL;
    scf_kat_result result;
    scf_size count;

    executed = 0;
    if (scf_provider_registry_create(&providers)
            != SCF_STATUS_SUCCESS
        || scf_provider_register(providers, &provider)
               != SCF_STATUS_SUCCESS
        || scf_provider_register(providers, &empty_provider)
               != SCF_STATUS_SUCCESS
        || scf_kat_registry_create(providers, &registry)
               != SCF_STATUS_SUCCESS
        || scf_kat_register(registry, &pass)
               != SCF_STATUS_SUCCESS
        || scf_kat_register(registry, &pass)
               != SCF_STATUS_DUPLICATE
        || scf_kat_register(registry, &invalid)
               != SCF_STATUS_TEST_INVALID
        || scf_kat_register(registry, &missing)
               != SCF_STATUS_NOT_FOUND
        || scf_kat_register(registry, &fail)
               != SCF_STATUS_SUCCESS
        || scf_kat_register(registry, &unsupported)
               != SCF_STATUS_SUCCESS
        || scf_kat_run(registry,
                       SCF_PROVIDER_HASH,
                       77,
                       1,
                       &result)
               != SCF_STATUS_SUCCESS
        || result.status != SCF_STATUS_SUCCESS
        || strcmp(result.provider_name, "kat-dummy") != 0
        || strcmp(result.test_name, "pass") != 0
        || scf_kat_run(registry,
                       SCF_PROVIDER_HASH,
                       77,
                       2,
                       &result)
               != SCF_STATUS_TEST_FAILED
        || result.status != SCF_STATUS_TEST_FAILED
        || scf_kat_run(registry,
                       SCF_PROVIDER_HASH,
                       77,
                       3,
                       &result)
               != SCF_STATUS_TEST_UNSUPPORTED
        || scf_kat_run(registry,
                       SCF_PROVIDER_HASH,
                       77,
                       999,
                       &result)
               != SCF_STATUS_NOT_FOUND
        || scf_kat_run_provider(registry,
                                SCF_PROVIDER_HASH,
                                77,
                                &result,
                                &count)
               != SCF_STATUS_TEST_UNSUPPORTED
        || count != 3 || executed != 6
        || scf_kat_run_provider(registry,
                                SCF_PROVIDER_HASH,
                                78,
                                &result,
                                &count)
               != SCF_STATUS_TEST_NO_VECTORS
        || count != 0
        || scf_kat_run_provider(registry,
                                SCF_PROVIDER_HASH,
                                999,
                                &result,
                                &count)
               != SCF_STATUS_NOT_FOUND
        || scf_kat_run_all(registry, &result, &count)
               != SCF_STATUS_TEST_UNSUPPORTED
        || count != 3
        || scf_kat_unregister(registry,
                              SCF_PROVIDER_HASH,
                              77,
                              1)
               != SCF_STATUS_SUCCESS
        || scf_kat_run_provider(registry,
                                SCF_PROVIDER_HASH,
                                77,
                                &result,
                                &count)
               != SCF_STATUS_TEST_UNSUPPORTED
        || count != 2)
    {
        scf_kat_registry_destroy(registry);
        scf_provider_registry_destroy(providers);
        return 1;
    }
    if (scf_kat_unregister(registry,
                           SCF_PROVIDER_HASH,
                           77,
                           1)
            != SCF_STATUS_NOT_FOUND
        || scf_kat_registry_destroy(registry)
               != SCF_STATUS_BUSY
        || scf_kat_unregister(registry,
                              SCF_PROVIDER_HASH,
                              77,
                              2)
               != SCF_STATUS_SUCCESS
        || scf_kat_unregister(registry,
                              SCF_PROVIDER_HASH,
                              77,
                              3)
               != SCF_STATUS_SUCCESS
        || scf_kat_registry_destroy(registry)
               != SCF_STATUS_SUCCESS
        || scf_provider_unregister(providers,
                                   SCF_PROVIDER_HASH,
                                   77)
               != SCF_STATUS_SUCCESS
        || scf_provider_unregister(providers,
                                   SCF_PROVIDER_HASH,
                                   78)
               != SCF_STATUS_SUCCESS
        || scf_provider_registry_destroy(providers)
               != SCF_STATUS_SUCCESS)
    {
        return 1;
    }
    return 0;
}
