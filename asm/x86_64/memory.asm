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
	testq %rsi, %rsi
	je 2f
	xorl %eax, %eax
1:
	movb %al, (%rdi)
	incq %rdi
	decq %rsi
	jne 1b
2:
	ret

.globl SCF_EQUAL
SCF_EQUAL:
	xorl %eax, %eax
	testq %rdx, %rdx
	je 2f
	xorl %ecx, %ecx
1:
	movzbl (%rdi), %r8d
	movzbl (%rsi), %r9d
	xorl %r9d, %r8d
	orl %r8d, %ecx
	incq %rdi
	incq %rsi
	decq %rdx
	jne 1b
	testl %ecx, %ecx
	sete %al
2:
	ret
