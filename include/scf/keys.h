#ifndef SCF_KEYS_H
#define SCF_KEYS_H

#include <scf/scf.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct scf_key scf_key;

    typedef enum
    {
        SCF_KEY_INITIALIZED = 1,
        SCF_KEY_CLEARED = 2
    } scf_key_state;

    SCF_API scf_status
    scf_key_create(scf_const_buffer material,
                   scf_key **key);
    SCF_API scf_status scf_key_copy(const scf_key *source,
                                    scf_key **copy);
    SCF_API scf_status
    scf_key_data(const scf_key *key,
                 scf_const_buffer *material);
    SCF_API scf_size scf_key_size(const scf_key *key);
    SCF_API scf_key_state
    scf_key_state_get(const scf_key *key);
    SCF_API scf_status scf_key_clear(scf_key *key);
    SCF_API void scf_key_destroy(scf_key *key);

#ifdef __cplusplus
}
#endif

#endif
