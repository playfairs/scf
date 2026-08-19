.text
.p2align 2
#ifdef SCF_DARWIN
#define SCF_COPY _scf_asm_copy
#else
#define SCF_COPY scf_asm_copy
#endif

.globl SCF_COPY
SCF_COPY:
	cbz x2, 2f
1:
	ldrb w3, [x1], #1
	strb w3, [x0], #1
	subs x2, x2, #1
	b.ne 1b
2:
	ret
