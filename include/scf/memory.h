#ifndef SCF_MEMORY_H
#define SCF_MEMORY_H

#include <scf/scf.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct scf_secure_allocation
        scf_secure_allocation;

    SCF_API scf_status
    scf_secure_allocate(scf_size size,
                        scf_size alignment,
                        scf_secure_allocation **allocation);
    SCF_API scf_status
    scf_secure_data(scf_secure_allocation *allocation,
                    scf_buffer *buffer);
    SCF_API scf_status
    scf_secure_size(const scf_secure_allocation *allocation,
                    scf_size *size);
    SCF_API scf_status
    scf_secure_clear(scf_secure_allocation *allocation);
    SCF_API void
    scf_secure_destroy(scf_secure_allocation *allocation);

#ifdef __cplusplus
}
#endif

#endif