.text
.p2align 2
#ifdef SCF_DARWIN
#define SCF_BYTESWAP _scf_asm_byteswap64
#define SCF_ROTL _scf_asm_rotl64
#else
#define SCF_BYTESWAP scf_asm_byteswap64
#define SCF_ROTL scf_asm_rotl64
#endif

.globl SCF_BYTESWAP
SCF_BYTESWAP:
	rev x0, x0
	ret

.globl SCF_ROTL
SCF_ROTL:
	neg x1, x1
	ror x0, x0, x1
	ret
