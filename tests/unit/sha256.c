#include <stdio.h>
#include <string.h>

#include <scf/kat.h>
#include <scf/sha256.h>

extern void scf_sha256_compress_c(uint32_t words[8],
                                  const scf_byte *blocks,
                                  scf_size block_count);
extern void scf_sha256_compress_asm(uint32_t words[8],
                                    const scf_byte *blocks,
                                    scf_size block_count);

static scf_status
sha256_vector(const scf_kat_vector *vector)
{
    scf_hash_context *context = NULL;
    scf_byte digest[SCF_SHA256_DIGEST_SIZE];
    scf_size written = 0;
    scf_size chunk = vector->auxiliary.size == 1
                         ? vector->auxiliary.data[0]
                         : vector->input.size;
    scf_size offset = 0;
    scf_status status;

    status =
        scf_hash_context_create(scf_sha256_hash_provider(),
                                &context);
    if (status != SCF_STATUS_SUCCESS)
    {
        return status;
    }
    while (offset < vector->input.size)
    {
        scf_size remaining = vector->input.size - offset;
        scf_size amount =
            remaining < chunk ? remaining : chunk;
        status = scf_hash_update(
            context,
            (scf_const_buffer){vector->input.data + offset,
                               amount});
        if (status != SCF_STATUS_SUCCESS)
        {
            scf_hash_context_destroy(context);
            return status;
        }
        offset += amount;
    }
    if (vector->input.size == 0)
    {
        status =
            scf_hash_update(context,
                            (scf_const_buffer){NULL, 0});
        if (status != SCF_STATUS_SUCCESS)
        {
            scf_hash_context_destroy(context);
            return status;
        }
    }
    status =
        scf_hash_final(context,
                       (scf_buffer){digest, sizeof(digest)},
                       &written);
    if (status == SCF_STATUS_SUCCESS
        && (written != vector->expected.size
            || memcmp(digest,
                      vector->expected.data,
                      written)
                   != 0))
    {
        status = SCF_STATUS_TEST_FAILED;
    }
    scf_hash_context_destroy(context);
    return status;
}

static int sha256_hex(const char *text, scf_byte output[32])
{
    for (scf_size index = 0; index < 32; ++index)
    {
        unsigned int value;
        if (sscanf(text + index * 2, "%2x", &value) != 1)
        {
            return 1;
        }
        output[index] = (scf_byte)value;
    }
    return 0;
}

static int sha256_boundary(scf_kat_registry *registry,
                           uint32_t identifier,
                           scf_size length,
                           const char *digest)
{
    scf_byte input[128];
    scf_byte expected[32];
    scf_kat_descriptor descriptor;

    memset(input, 'a', length);
    if (sha256_hex(digest, expected) != 0)
    {
        return 1;
    }
    descriptor =
        (scf_kat_descriptor){SCF_PROVIDER_HASH,
                             SCF_SHA256_PROVIDER_ID,
                             identifier,
                             "boundary",
                             {{input, length},
                              {expected, sizeof(expected)},
                              {NULL, 0},
                              {NULL, 0},
                              {NULL, 0},
                              {NULL, 0}},
                             sha256_vector};
    return scf_kat_register(registry, &descriptor)
                   == SCF_STATUS_SUCCESS
               ? 0
               : 1;
}

int scf_unit_sha256(void)
{
    static const scf_byte empty[] = "";
    static const scf_byte abc[] = "abc";
    static const scf_byte quick[] =
        "The quick brown fox jumps over the lazy dog";
    static const scf_byte multi[] =
        "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomn"
        "opnopq";
    static const scf_byte empty_digest[] = {
        0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14,
        0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
        0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
        0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55};
    static const scf_byte abc_digest[] = {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
        0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
        0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
        0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad};
    static const scf_byte quick_digest[] = {
        0xd7, 0xa8, 0xfb, 0xb3, 0x07, 0xd7, 0x80, 0x94,
        0x69, 0xca, 0x9a, 0xbc, 0xb0, 0x08, 0x2e, 0x4f,
        0x8d, 0x56, 0x51, 0xe4, 0x6d, 0x3c, 0xdb, 0x76,
        0x2d, 0x02, 0xd0, 0xbf, 0x37, 0xc9, 0xe5, 0x92};
    static const scf_byte multi_digest[] = {
        0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
        0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
        0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
        0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1};
    scf_provider_registry *providers = NULL;
    scf_kat_registry *registry = NULL;
    scf_kat_result result;
    scf_size executed = 0;
    scf_status invalid_context_status;
    scf_status all_status;
    uint32_t words_c[8] = {0x6a09e667,
                           0xbb67ae85,
                           0x3c6ef372,
                           0xa54ff53a,
                           0x510e527f,
                           0x9b05688c,
                           0x1f83d9ab,
                           0x5be0cd19};
    uint32_t words_asm[8];
    scf_byte block[64] = {0};

    memcpy(words_asm, words_c, sizeof(words_c));
    block[0] = 0x61;
    block[1] = 0x62;
    block[2] = 0x63;
    block[3] = 0x80;
    block[63] = 24;
    scf_sha256_compress_c(words_c, block, 1);
    scf_sha256_compress_asm(words_asm, block, 1);
    if (memcmp(words_c, words_asm, sizeof(words_c)) != 0)
    {
        return 1;
    }
    for (uint32_t trial = 0; trial < 32; ++trial)
    {
        uint32_t seed = UINT32_C(0x9e3779b9) ^ trial;
        scf_byte blocks[256];
        uint32_t reference[8] = {0x6a09e667,
                                 0xbb67ae85,
                                 0x3c6ef372,
                                 0xa54ff53a,
                                 0x510e527f,
                                 0x9b05688c,
                                 0x1f83d9ab,
                                 0x5be0cd19};
        uint32_t backend[8];
        scf_size block_count = (trial % 4u) + 1u;

        for (scf_size index = 0;
             index < block_count * SCF_SHA256_BLOCK_SIZE;
             ++index)
        {
            seed = seed * UINT32_C(1664525)
                   + UINT32_C(1013904223);
            blocks[index] = (scf_byte)(seed >> 24);
        }
        memcpy(backend, reference, sizeof(reference));
        scf_sha256_compress_c(reference,
                              blocks,
                              block_count);
        scf_sha256_compress_asm(backend,
                                blocks,
                                block_count);
        if (memcmp(reference, backend, sizeof(reference))
            != 0)
        {
            return 1;
        }
    }
    if (scf_provider_registry_create(&providers)
            != SCF_STATUS_SUCCESS
        || scf_sha256_register(providers)
               != SCF_STATUS_SUCCESS
        || scf_kat_registry_create(providers, &registry)
               != SCF_STATUS_SUCCESS)
    {
        scf_kat_registry_destroy(registry);
        scf_provider_registry_destroy(providers);
        return 1;
    }
    const scf_kat_descriptor vectors[] = {
        {SCF_PROVIDER_HASH,
         SCF_SHA256_PROVIDER_ID,
         1,
         "empty",
         {{empty, 0},
          {empty_digest, sizeof(empty_digest)},
          {NULL, 0},
          {NULL, 0},
          {NULL, 0},
          {NULL, 0}},
         sha256_vector},
        {SCF_PROVIDER_HASH,
         SCF_SHA256_PROVIDER_ID,
         2,
         "abc",
         {{abc, sizeof(abc) - 1},
          {abc_digest, sizeof(abc_digest)},
          {NULL, 0},
          {NULL, 0},
          {NULL, 0},
          {(const scf_byte[]){1}, 1}},
         sha256_vector},
        {SCF_PROVIDER_HASH,
         SCF_SHA256_PROVIDER_ID,
         3,
         "quick",
         {{quick, sizeof(quick) - 1},
          {quick_digest, sizeof(quick_digest)},
          {NULL, 0},
          {NULL, 0},
          {NULL, 0},
          {(const scf_byte[]){7}, 1}},
         sha256_vector},
        {SCF_PROVIDER_HASH,
         SCF_SHA256_PROVIDER_ID,
         4,
         "multi",
         {{multi, sizeof(multi) - 1},
          {multi_digest, sizeof(multi_digest)},
          {NULL, 0},
          {NULL, 0},
          {NULL, 0},
          {(const scf_byte[]){13}, 1}},
         sha256_vector}};

    for (scf_size index = 0;
         index < sizeof(vectors) / sizeof(vectors[0]);
         ++index)
    {
        if (scf_kat_register(registry, &vectors[index])
            != SCF_STATUS_SUCCESS)
        {
            scf_kat_registry_destroy(registry);
            scf_provider_registry_destroy(providers);
            return 1;
        }
    }
    memset(&result, 0, sizeof(result));
    invalid_context_status =
        scf_hash_context_create(scf_sha256_hash_provider(),
                                NULL);
    if (sha256_boundary(registry,
                        5,
                        55,
                        "9f4390f8d30c2dd92ec9f095b65e2b9ae9"
                        "b0a925a5258e241c9f1e910f734318")
            != 0
        || sha256_boundary(
               registry,
               6,
               56,
               "b35439a4ac6f0948b6d6f9e3c6af0f5f590ce20f1bd"
               "e7090ef7970686ec6738a")
               != 0
        || sha256_boundary(
               registry,
               7,
               63,
               "7d3e74a05d7db15bce4ad9ec0658ea98e3f06eeecf1"
               "6b4c6fff2da457ddc2f34")
               != 0
        || sha256_boundary(
               registry,
               8,
               64,
               "ffe054fe7ae0cb6dc65c3af9b61d5209f439851db43"
               "d0ba5997337df154668eb")
               != 0
        || sha256_boundary(
               registry,
               9,
               65,
               "635361c48bb9eab14198e76ea8ab7f1a41685d6ad62"
               "aa9146d301d4f17eb0ae0")
               != 0)
    {
        fprintf(stderr,
                "sha256 KAT registration failure\n");
        scf_kat_registry_destroy(registry);
        scf_provider_registry_destroy(providers);
        return 1;
    }
    all_status =
        scf_kat_run_all(registry, &result, &executed);
    if (all_status != SCF_STATUS_SUCCESS || executed != 9
        || invalid_context_status
               != SCF_STATUS_INVALID_ARGUMENT)
    {
        scf_kat_registry_destroy(registry);
        scf_provider_registry_destroy(providers);
        return 1;
    }
    scf_kat_registry_destroy(registry);
    scf_provider_registry_destroy(providers);
    return 0;
}
