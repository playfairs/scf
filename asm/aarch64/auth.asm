.text
.p2align 2
#ifdef SCF_DARWIN
#define SCF_SELECT _scf_asm_select64
#else
#define SCF_SELECT scf_asm_select64
#endif

.globl SCF_SELECT
SCF_SELECT:
	cmp x2, #0
	csel x0, x0, x1, ne
	ret
