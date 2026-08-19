#ifndef SCF_SHA256_INTERNAL_H
#define SCF_SHA256_INTERNAL_H

#include <scf/scf.h>

typedef struct
{
    uint32_t words[8];
    uint64_t bit_length;
    scf_byte block[64];
    scf_size block_length;
} scf_sha256_state;

void scf_sha256_compress_c(uint32_t words[8],
                           const scf_byte *blocks,
                           scf_size block_count);
void scf_sha256_compress_asm(uint32_t words[8],
                             const scf_byte *blocks,
                             scf_size block_count);

#endif