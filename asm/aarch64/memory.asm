.text
.p2align 2
#ifdef SCF_DARWIN
#define SCF_SECURE_CLEAR _scf_asm_secure_clear
#define SCF_EQUAL _scf_asm_equal
#else
#define SCF_SECURE_CLEAR scf_asm_secure_clear
#define SCF_EQUAL scf_asm_equal
#endif

.globl SCF_SECURE_CLEAR
SCF_SECURE_CLEAR:
	cbz x1, 2f
	mov w3, wzr
1:
	strb w3, [x0], #1
	subs x1, x1, #1
	b.ne 1b
2:
	ret

.globl SCF_EQUAL
SCF_EQUAL:
	mov w3, wzr
	cbz x2, 2f
1:
	ldrb w4, [x0], #1
	ldrb w5, [x1], #1
	eor w4, w4, w5
	orr w3, w3, w4
	subs x2, x2, #1
	b.ne 1b
	cmp w3, wzr
	cset w0, eq
	ret
2:
	mov w0, #1
	ret
