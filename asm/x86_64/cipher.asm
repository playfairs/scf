.text
.p2align 2
#ifdef SCF_DARWIN
#define SCF_COPY _scf_asm_copy
#else
#define SCF_COPY scf_asm_copy
#endif

.globl SCF_COPY
SCF_COPY:
	testq %rdx, %rdx
	je 2f
1:
	movb (%rsi), %al
	movb %al, (%rdi)
	incq %rdi
	incq %rsi
	decq %rdx
	jne 1b
2:
	ret
