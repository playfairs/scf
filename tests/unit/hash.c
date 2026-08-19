#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <scf/hash.h>
#include <scf/scf.h>

typedef struct
{
    uint64_t total;
} hash_state;

static scf_status hash_init(void **state)
{
    *state = calloc(1, sizeof(hash_state));
    return *state == NULL ? SCF_STATUS_ALLOCATION_FAILED
                          : SCF_STATUS_SUCCESS;
}

static scf_status hash_update(void *state,
                              scf_const_buffer input)
{
    hash_state *hash = state;
    for (scf_size index = 0; index < input.size; ++index)
    {
        hash->total += input.data[index];
    }
    return SCF_STATUS_SUCCESS;
}

static scf_status hash_final(void *state,
                             scf_buffer output,
                             scf_size *written)
{
    hash_state *hash = state;
    if (output.size < sizeof(hash->total))
    {
        return SCF_STATUS_BUFFER_TOO_SMALL;
    }
    memcpy(output.data, &hash->total, sizeof(hash->total));
    *written = sizeof(hash->total);
    return SCF_STATUS_SUCCESS;
}

static scf_status hash_reset(void *state)
{
    ((hash_state *)state)->total = 0;
    return SCF_STATUS_SUCCESS;
}

static void hash_destroy(void *state)
{
    free(state);
}

int scf_unit_hash(void)
{
    scf_hash_provider provider = {"test",
                                  8,
                                  8,
                                  hash_init,
                                  hash_update,
                                  hash_final,
                                  hash_reset,
                                  hash_destroy};
    scf_hash_context *context = NULL;
    scf_byte input[] = {1, 2, 3};
    scf_byte output[8] = {0};
    scf_size written = 0;
    uint64_t expected = 6;

    if (scf_hash_context_create(&provider, &context)
            != SCF_STATUS_SUCCESS
        || scf_hash_update(
               context,
               (scf_const_buffer){input, sizeof(input)})
               != SCF_STATUS_SUCCESS
        || scf_hash_final(
               context,
               (scf_buffer){output, sizeof(output)},
               &written)
               != SCF_STATUS_SUCCESS
        || written != sizeof(expected))
    {
        scf_hash_context_destroy(context);
        return 1;
    }
    memcpy(&expected, output, sizeof(expected));
    scf_hash_context_destroy(context);
    return expected == 6 ? 0 : 1;
}
