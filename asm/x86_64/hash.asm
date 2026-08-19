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
	movq %rdi, %rax
	bswapq %rax
	ret

.globl SCF_ROTL
SCF_ROTL:
	movq %rdi, %rax
	movb %sil, %cl
	rolq %cl, %rax
	ret
