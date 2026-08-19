#ifndef SCF_SHA256_H
#define SCF_SHA256_H

#include <scf/hash.h>
#include <scf/provider.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SCF_SHA256_PROVIDER_ID 1u
#define SCF_SHA256_DIGEST_SIZE 32u
#define SCF_SHA256_BLOCK_SIZE 64u

    SCF_API const scf_hash_provider *
    scf_sha256_hash_provider(void);
    SCF_API scf_status
    scf_sha256_register(scf_provider_registry *registry);

#ifdef __cplusplus
}
#endif

#endif