#ifndef SCF_HASH_H
#define SCF_HASH_H

#include <scf/scf.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct scf_hash_context scf_hash_context;

    typedef scf_status (*scf_hash_init_fn)(void **state);
    typedef scf_status (*scf_hash_update_fn)(
        void *state,
        scf_const_buffer input);
    typedef scf_status (*scf_hash_final_fn)(
        void *state,
        scf_buffer output,
        scf_size *written);
    typedef scf_status (*scf_hash_reset_fn)(void *state);
    typedef void (*scf_hash_destroy_fn)(void *state);

    typedef struct
    {
        const char *name;
        scf_size digest_size;
        scf_size block_size;
        scf_hash_init_fn init;
        scf_hash_update_fn update;
        scf_hash_final_fn final;
        scf_hash_reset_fn reset;
        scf_hash_destroy_fn destroy;
    } scf_hash_provider;

    SCF_API scf_status scf_hash_context_create(
        const scf_hash_provider *provider,
        scf_hash_context **context);
    SCF_API scf_status
    scf_hash_update(scf_hash_context *context,
                    scf_const_buffer input);
    SCF_API scf_status
    scf_hash_final(scf_hash_context *context,
                   scf_buffer output,
                   scf_size *written);
    SCF_API scf_status
    scf_hash_reset(scf_hash_context *context);
    SCF_API scf_size
    scf_hash_digest_size(const scf_hash_context *context);
    SCF_API scf_size
    scf_hash_block_size(const scf_hash_context *context);
    SCF_API void
    scf_hash_context_destroy(scf_hash_context *context);

#ifdef __cplusplus
}
#endif

#endif
