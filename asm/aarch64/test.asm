.text
.p2align 2
#ifdef SCF_DARWIN
.globl _scf_asm_test
_scf_asm_test:
#else
.globl scf_asm_test
scf_asm_test:
#endif
    add x0, x0, #1
    ret
