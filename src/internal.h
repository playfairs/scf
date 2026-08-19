#ifndef SCF_INTERNAL_H
#define SCF_INTERNAL_H

#include <stdlib.h>
#include <string.h>

#include <scf/cipher.h>
#include <scf/format.h>
#include <scf/hash.h>
#include <scf/kdf.h>
#include <scf/keys.h>
#include <scf/memory.h>
#include <scf/provider.h>

void *scf_internal_alloc(scf_size size);
void scf_internal_free(void *memory);
int scf_internal_buffer_valid(scf_const_buffer buffer);
int scf_internal_output_valid(scf_buffer buffer);
void scf_internal_clear(void *memory, scf_size size);
void scf_internal_copy(void *destination, const void *source, scf_size size);
int scf_internal_equal(const void *left, const void *right, scf_size size);
uint64_t scf_internal_byteswap64(uint64_t value);
uint64_t scf_internal_select64(uint64_t when_true, uint64_t when_false, uint64_t condition);

#endif
