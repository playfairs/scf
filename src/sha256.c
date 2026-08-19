#include <stdint.h>

#include <scf/sha256.h>

#include "internal.h"
#include "sha256_internal.h"

extern void scf_asm_sha256_compress(uint32_t words[8],
                                    const scf_byte *blocks,
                                    scf_size block_count);

static const uint32_t scf_sha256_rounds[64] = {
    UINT32_C(0x428a2f98), UINT32_C(0x71374491),
    UINT32_C(0xb5c0fbcf), UINT32_C(0xe9b5dba5),
    UINT32_C(0x3956c25b), UINT32_C(0x59f111f1),
    UINT32_C(0x923f82a4), UINT32_C(0xab1c5ed5),
    UINT32_C(0xd807aa98), UINT32_C(0x12835b01),
    UINT32_C(0x243185be), UINT32_C(0x550c7dc3),
    UINT32_C(0x72be5d74), UINT32_C(0x80deb1fe),
    UINT32_C(0x9bdc06a7), UINT32_C(0xc19bf174),
    UINT32_C(0xe49b69c1), UINT32_C(0xefbe4786),
    UINT32_C(0x0fc19dc6), UINT32_C(0x240ca1cc),
    UINT32_C(0x2de92c6f), UINT32_C(0x4a7484aa),
    UINT32_C(0x5cb0a9dc), UINT32_C(0x76f988da),
    UINT32_C(0x983e5152), UINT32_C(0xa831c66d),
    UINT32_C(0xb00327c8), UINT32_C(0xbf597fc7),
    UINT32_C(0xc6e00bf3), UINT32_C(0xd5a79147),
    UINT32_C(0x06ca6351), UINT32_C(0x14292967),
    UINT32_C(0x27b70a85), UINT32_C(0x2e1b2138),
    UINT32_C(0x4d2c6dfc), UINT32_C(0x53380d13),
    UINT32_C(0x650a7354), UINT32_C(0x766a0abb),
    UINT32_C(0x81c2c92e), UINT32_C(0x92722c85),
    UINT32_C(0xa2bfe8a1), UINT32_C(0xa81a664b),
    UINT32_C(0xc24b8b70), UINT32_C(0xc76c51a3),
    UINT32_C(0xd192e819), UINT32_C(0xd6990624),
    UINT32_C(0xf40e3585), UINT32_C(0x106aa070),
    UINT32_C(0x19a4c116), UINT32_C(0x1e376c08),
    UINT32_C(0x2748774c), UINT32_C(0x34b0bcb5),
    UINT32_C(0x391c0cb3), UINT32_C(0x4ed8aa4a),
    UINT32_C(0x5b9cca4f), UINT32_C(0x682e6ff3),
    UINT32_C(0x748f82ee), UINT32_C(0x78a5636f),
    UINT32_C(0x84c87814), UINT32_C(0x8cc70208),
    UINT32_C(0x90befffa), UINT32_C(0xa4506ceb),
    UINT32_C(0xbef9a3f7), UINT32_C(0xc67178f2)};

static uint32_t scf_sha256_rotr(uint32_t value,
                                uint32_t amount)
{
    return (value >> amount) | (value << (32u - amount));
}

static uint32_t scf_sha256_load32(const scf_byte *input)
{
    return ((uint32_t)input[0] << 24)
           | ((uint32_t)input[1] << 16)
           | ((uint32_t)input[2] << 8) | input[3];
}

void scf_sha256_compress_c(uint32_t words[8],
                           const scf_byte *blocks,
                           scf_size block_count)
{
    for (scf_size block = 0; block < block_count; ++block)
    {
        uint32_t schedule[64];
        uint32_t state[8];
        const scf_byte *input = blocks + block * 64u;

        for (scf_size index = 0; index < 16; ++index)
        {
            schedule[index] =
                scf_sha256_load32(input + index * 4u);
        }
        for (scf_size index = 16; index < 64; ++index)
        {
            uint32_t small_sigma0 =
                scf_sha256_rotr(schedule[index - 15], 7)
                ^ scf_sha256_rotr(schedule[index - 15], 18)
                ^ (schedule[index - 15] >> 3);
            uint32_t small_sigma1 =
                scf_sha256_rotr(schedule[index - 2], 17)
                ^ scf_sha256_rotr(schedule[index - 2], 19)
                ^ (schedule[index - 2] >> 10);
            schedule[index] =
                schedule[index - 16] + small_sigma0
                + schedule[index - 7] + small_sigma1;
        }
        for (scf_size index = 0; index < 8; ++index)
        {
            state[index] = words[index];
        }
        for (scf_size index = 0; index < 64; ++index)
        {
            uint32_t big_sigma1 =
                scf_sha256_rotr(state[4], 6)
                ^ scf_sha256_rotr(state[4], 11)
                ^ scf_sha256_rotr(state[4], 25);
            uint32_t choose = (state[4] & state[5])
                              ^ (~state[4] & state[6]);
            uint32_t temporary1 = state[7] + big_sigma1
                                  + choose
                                  + scf_sha256_rounds[index]
                                  + schedule[index];
            uint32_t big_sigma0 =
                scf_sha256_rotr(state[0], 2)
                ^ scf_sha256_rotr(state[0], 13)
                ^ scf_sha256_rotr(state[0], 22);
            uint32_t majority = (state[0] & state[1])
                                ^ (state[0] & state[2])
                                ^ (state[1] & state[2]);
            uint32_t temporary2 = big_sigma0 + majority;

            state[7] = state[6];
            state[6] = state[5];
            state[5] = state[4];
            state[4] = state[3] + temporary1;
            state[3] = state[2];
            state[2] = state[1];
            state[1] = state[0];
            state[0] = temporary1 + temporary2;
        }
        for (scf_size index = 0; index < 8; ++index)
        {
            words[index] += state[index];
        }
        scf_internal_clear(schedule, sizeof(schedule));
        scf_internal_clear(state, sizeof(state));
    }
}

void scf_sha256_compress_asm(uint32_t words[8],
                             const scf_byte *blocks,
                             scf_size block_count)
{
    scf_asm_sha256_compress(words, blocks, block_count);
}

static void scf_sha256_initialize(scf_sha256_state *state)
{
    state->words[0] = UINT32_C(0x6a09e667);
    state->words[1] = UINT32_C(0xbb67ae85);
    state->words[2] = UINT32_C(0x3c6ef372);
    state->words[3] = UINT32_C(0xa54ff53a);
    state->words[4] = UINT32_C(0x510e527f);
    state->words[5] = UINT32_C(0x9b05688c);
    state->words[6] = UINT32_C(0x1f83d9ab);
    state->words[7] = UINT32_C(0x5be0cd19);
    state->bit_length = 0;
    state->block_length = 0;
    scf_internal_clear(state->block, sizeof(state->block));
}

static scf_status scf_sha256_init(void **state)
{
    scf_secure_allocation *allocation = NULL;
    scf_buffer buffer;
    scf_status status =
        scf_secure_allocate(sizeof(scf_sha256_state),
                            _Alignof(scf_sha256_state),
                            &allocation);

    if (status != SCF_STATUS_SUCCESS)
    {
        return status;
    }
    if (scf_secure_data(allocation, &buffer)
        != SCF_STATUS_SUCCESS)
    {
        scf_secure_destroy(allocation);
        return SCF_STATUS_INTERNAL_FAILURE;
    }
    scf_sha256_initialize((scf_sha256_state *)buffer.data);
    *state = allocation;
    return SCF_STATUS_SUCCESS;
}

static scf_sha256_state *scf_sha256_state_data(void *state)
{
    scf_secure_allocation *allocation = state;
    scf_buffer buffer;

    if (allocation == NULL
        || scf_secure_data(allocation, &buffer)
               != SCF_STATUS_SUCCESS)
    {
        return NULL;
    }
    return (scf_sha256_state *)buffer.data;
}

static scf_status scf_sha256_update(void *state,
                                    scf_const_buffer input)
{
    scf_sha256_state *sha = scf_sha256_state_data(state);
    scf_size offset = 0;

    if (sha == NULL || !scf_internal_buffer_valid(input))
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    if (input.size > (UINT64_MAX - sha->bit_length) / 8u)
    {
        return SCF_STATUS_OVERFLOW;
    }
    sha->bit_length += (uint64_t)input.size * 8u;
    while (offset < input.size)
    {
        scf_size available =
            sizeof(sha->block) - sha->block_length;
        scf_size remaining = input.size - offset;
        scf_size amount =
            available < remaining ? available : remaining;

        scf_internal_copy(sha->block + sha->block_length,
                          input.data + offset,
                          amount);
        sha->block_length += amount;
        offset += amount;
        if (sha->block_length == sizeof(sha->block))
        {
            scf_sha256_compress_asm(sha->words,
                                    sha->block,
                                    1);
            sha->block_length = 0;
        }
    }
    return SCF_STATUS_SUCCESS;
}

static void scf_sha256_store32(scf_byte *output,
                               uint32_t value)
{
    output[0] = (scf_byte)(value >> 24);
    output[1] = (scf_byte)(value >> 16);
    output[2] = (scf_byte)(value >> 8);
    output[3] = (scf_byte)value;
}

static scf_status scf_sha256_final(void *state,
                                   scf_buffer output,
                                   scf_size *written)
{
    scf_sha256_state *sha = scf_sha256_state_data(state);
    scf_sha256_state final_state;
    uint64_t bit_length;

    if (sha == NULL || written == NULL
        || !scf_internal_output_valid(output))
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    if (output.size < SCF_SHA256_DIGEST_SIZE)
    {
        return SCF_STATUS_BUFFER_TOO_SMALL;
    }
    final_state = *sha;
    final_state.block[final_state.block_length++] = 0x80;
    if (final_state.block_length > 56)
    {
        scf_internal_clear(final_state.block
                               + final_state.block_length,
                           sizeof(final_state.block)
                               - final_state.block_length);
        scf_sha256_compress_asm(final_state.words,
                                final_state.block,
                                1);
        final_state.block_length = 0;
    }
    scf_internal_clear(final_state.block
                           + final_state.block_length,
                       56 - final_state.block_length);
    bit_length = final_state.bit_length;
    for (scf_size index = 0; index < 8; ++index)
    {
        final_state.block[56 + index] =
            (scf_byte)(bit_length >> (56 - index * 8));
    }
    scf_sha256_compress_asm(final_state.words,
                            final_state.block,
                            1);
    for (scf_size index = 0; index < 8; ++index)
    {
        scf_sha256_store32(output.data + index * 4,
                           final_state.words[index]);
    }
    *written = SCF_SHA256_DIGEST_SIZE;
    scf_internal_clear(&final_state, sizeof(final_state));
    return SCF_STATUS_SUCCESS;
}

static scf_status scf_sha256_reset(void *state)
{
    scf_sha256_state *sha = scf_sha256_state_data(state);

    if (sha == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    scf_sha256_initialize(sha);
    return SCF_STATUS_SUCCESS;
}

static void scf_sha256_destroy(void *state)
{
    scf_secure_destroy(state);
}

static const scf_hash_provider scf_sha256_provider = {
    "SHA-256",
    SCF_SHA256_DIGEST_SIZE,
    SCF_SHA256_BLOCK_SIZE,
    scf_sha256_init,
    scf_sha256_update,
    scf_sha256_final,
    scf_sha256_reset,
    scf_sha256_destroy};

static scf_status
scf_sha256_context_create(scf_const_buffer parameters,
                          void **state)
{
    (void)parameters;
    return scf_sha256_init(state);
}

static void scf_sha256_context_destroy(void *state)
{
    scf_sha256_destroy(state);
}

const scf_hash_provider *scf_sha256_hash_provider(void)
{
    return &scf_sha256_provider;
}

scf_status
scf_sha256_register(scf_provider_registry *registry)
{
    const scf_provider_descriptor descriptor = {
        SCF_PROVIDER_HASH,
        SCF_SHA256_PROVIDER_ID,
        "SHA-256",
        SCF_PROVIDER_CAP_CONTEXT
            | SCF_PROVIDER_CAP_STREAMING,
        scf_sha256_context_create,
        scf_sha256_context_destroy};

    return scf_provider_register(registry, &descriptor);
}