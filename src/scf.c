#include <scf/scf.h>
#include <scf/sha256.h>

#include "internal.h"

extern uint64_t scf_asm_test(uint64_t value);
extern void scf_asm_secure_clear(void *memory,
                                 scf_size size);
extern int scf_asm_equal(const void *left,
                         const void *right,
                         scf_size size);
extern void scf_asm_copy(void *destination,
                         const void *source,
                         scf_size size);
extern uint64_t scf_asm_byteswap64(uint64_t value);
extern uint64_t scf_asm_select64(uint64_t when_true,
                                 uint64_t when_false,
                                 uint64_t condition);
extern uint64_t scf_asm_rotl64(uint64_t value,
                               uint64_t amount);

struct scf_context
{
    uint32_t active;
};

void *scf_internal_alloc(scf_size size)
{
    return size == 0 ? NULL : malloc(size);
}

void scf_internal_free(void *memory)
{
    free(memory);
}

int scf_internal_buffer_valid(scf_const_buffer buffer)
{
    return buffer.size == 0 || buffer.data != NULL;
}

int scf_internal_output_valid(scf_buffer buffer)
{
    return buffer.size == 0 || buffer.data != NULL;
}

void scf_internal_clear(void *memory, scf_size size)
{
    scf_asm_secure_clear(memory, size);
}

void scf_internal_copy(void *destination,
                       const void *source,
                       scf_size size)
{
    scf_asm_copy(destination, source, size);
}

int scf_internal_equal(const void *left,
                       const void *right,
                       scf_size size)
{
    return scf_asm_equal(left, right, size);
}

uint64_t scf_internal_byteswap64(uint64_t value)
{
    return scf_asm_byteswap64(value);
}

uint64_t scf_internal_select64(uint64_t when_true,
                               uint64_t when_false,
                               uint64_t condition)
{
    return scf_asm_select64(when_true,
                            when_false,
                            condition);
}

uint64_t scf_test(uint64_t value)
{
    return scf_asm_test(value);
}

scf_status scf_init(void)
{
    return scf_self_test();
}

scf_status scf_version(scf_version_info *version)
{
    if (version == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }

    version->major = SCF_VERSION_MAJOR;
    version->minor = SCF_VERSION_MINOR;
    version->patch = SCF_VERSION_PATCH;
    version->text = SCF_VERSION;
    return SCF_STATUS_SUCCESS;
}

scf_status scf_runtime(scf_runtime_info *runtime)
{
    if (runtime == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }

#if defined(__x86_64__) || defined(_M_X64)
    scf_internal_copy(runtime->architecture, "x86_64", 7);
#elif defined(__aarch64__) || defined(_M_ARM64)
    scf_internal_copy(runtime->architecture, "aarch64", 8);
#else
    return SCF_STATUS_UNSUPPORTED;
#endif
    runtime
        ->architecture[sizeof(runtime->architecture) - 1] =
        '\0';
    runtime->word_size = sizeof(void *);
    runtime->assembly_available = 1;
    return SCF_STATUS_SUCCESS;
}

const char *scf_status_name(scf_status status)
{
    switch (status)
    {
    case SCF_STATUS_SUCCESS:
        return "success";
    case SCF_STATUS_INVALID_ARGUMENT:
        return "invalid argument";
    case SCF_STATUS_INVALID_STATE:
        return "invalid state";
    case SCF_STATUS_BUFFER_TOO_SMALL:
        return "buffer too small";
    case SCF_STATUS_ALLOCATION_FAILED:
        return "allocation failed";
    case SCF_STATUS_UNSUPPORTED:
        return "unsupported";
    case SCF_STATUS_OVERFLOW:
        return "overflow";
    case SCF_STATUS_FORMAT_INVALID:
        return "invalid format";
    case SCF_STATUS_DUPLICATE:
        return "duplicate";
    case SCF_STATUS_NOT_FOUND:
        return "not found";
    case SCF_STATUS_BUSY:
        return "busy";
    case SCF_STATUS_NAME_TOO_LONG:
        return "name too long";
    case SCF_STATUS_TEST_FAILED:
        return "test failed";
    case SCF_STATUS_TEST_INVALID:
        return "invalid test";
    case SCF_STATUS_TEST_UNSUPPORTED:
        return "unsupported test";
    case SCF_STATUS_INTERNAL_FAILURE:
        return "internal failure";
    case SCF_STATUS_TEST_NO_VECTORS:
        return "test has no vectors";
    default:
        return "unknown status";
    }
}

scf_status scf_context_create(scf_context **context)
{
    if (context == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }

    *context = scf_internal_alloc(sizeof(**context));
    if (*context == NULL)
    {
        return SCF_STATUS_ALLOCATION_FAILED;
    }
    (*context)->active = 1;
    return SCF_STATUS_SUCCESS;
}

scf_status scf_context_reset(scf_context *context)
{
    if (context == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    context->active = 1;
    return SCF_STATUS_SUCCESS;
}

void scf_context_destroy(scf_context *context)
{
    if (context != NULL)
    {
        scf_internal_clear(context, sizeof(*context));
        scf_internal_free(context);
    }
}

scf_status scf_self_test(void)
{
    scf_byte source[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    scf_byte copied[8] = {0};
    uint64_t word = UINT64_C(0x0102030405060708);
    scf_provider_registry *providers = NULL;
    scf_hash_context *hash = NULL;
    scf_byte digest[SCF_SHA256_DIGEST_SIZE];
    scf_size written = 0;
    static const scf_byte abc[] = "abc";
    static const scf_byte abc_digest[] = {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
        0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
        0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
        0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad};

    if (scf_test(UINT64_C(41)) != UINT64_C(42))
    {
        return SCF_STATUS_INVALID_STATE;
    }
    scf_internal_copy(copied, source, sizeof(source));
    if (!scf_internal_equal(source, copied, sizeof(source)))
    {
        return SCF_STATUS_INVALID_STATE;
    }
    scf_internal_clear(copied, sizeof(copied));
    if (!scf_internal_equal(copied,
                            (scf_byte[8]){0},
                            sizeof(copied)))
    {
        return SCF_STATUS_INVALID_STATE;
    }
    if (scf_internal_byteswap64(word)
        != UINT64_C(0x0807060504030201))
    {
        return SCF_STATUS_INVALID_STATE;
    }
    if (scf_asm_rotl64(UINT64_C(1), UINT64_C(8))
        != UINT64_C(256))
    {
        return SCF_STATUS_INVALID_STATE;
    }
    if (scf_internal_select64(9, 3, 1) != 9
        || scf_internal_select64(9, 3, 2) != 9
        || scf_internal_select64(9, 3, 0) != 3)
    {
        return SCF_STATUS_INVALID_STATE;
    }
    if (scf_provider_registry_create(&providers)
            != SCF_STATUS_SUCCESS
        || scf_sha256_register(providers)
               != SCF_STATUS_SUCCESS
        || scf_hash_context_create(
               scf_sha256_hash_provider(),
               &hash)
               != SCF_STATUS_SUCCESS
        || scf_hash_update(
               hash,
               (scf_const_buffer){abc, sizeof(abc) - 1})
               != SCF_STATUS_SUCCESS
        || scf_hash_final(
               hash,
               (scf_buffer){digest, sizeof(digest)},
               &written)
               != SCF_STATUS_SUCCESS
        || written != sizeof(abc_digest)
        || !scf_internal_equal(digest,
                               abc_digest,
                               sizeof(abc_digest)))
    {
        scf_hash_context_destroy(hash);
        scf_provider_registry_destroy(providers);
        return SCF_STATUS_INVALID_STATE;
    }
    scf_hash_context_destroy(hash);
    if (scf_provider_unregister(providers,
                                SCF_PROVIDER_HASH,
                                SCF_SHA256_PROVIDER_ID)
            != SCF_STATUS_SUCCESS
        || scf_provider_registry_destroy(providers)
               != SCF_STATUS_SUCCESS)
    {
        return SCF_STATUS_INVALID_STATE;
    }
    return SCF_STATUS_SUCCESS;
}