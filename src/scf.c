#include <scf/scf.h>

extern uint64_t scf_asm_test(uint64_t value);

uint64_t scf_test(uint64_t value)
{
    return scf_asm_test(value);
}