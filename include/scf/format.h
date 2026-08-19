#ifndef SCF_FORMAT_H
#define SCF_FORMAT_H

#include <scf/scf.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCF_FORMAT_HEADER_SIZE 20

typedef struct {
    scf_byte magic[4];
    uint32_t version;
    uint32_t type;
    uint64_t payload_size;
} scf_format_header;

typedef struct scf_format_context scf_format_context;

SCF_API scf_status scf_format_context_create(scf_format_context **context);
SCF_API scf_status scf_format_header_set(scf_format_context *context, const scf_format_header *header);
SCF_API scf_status scf_format_header_get(const scf_format_context *context, scf_format_header *header);
SCF_API scf_status scf_format_validate(const scf_format_header *header);
SCF_API scf_status scf_format_serialize(const scf_format_header *header, scf_buffer output, scf_size *written);
SCF_API scf_status scf_format_deserialize(scf_const_buffer input, scf_format_header *header);
SCF_API void scf_format_context_destroy(scf_format_context *context);

#ifdef __cplusplus
}
#endif

#endif
