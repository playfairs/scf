.text
.p2align 2
#ifdef SCF_DARWIN
#define SCF_SELECT _scf_asm_select64
#else
#define SCF_SELECT scf_asm_select64
#endif

.globl SCF_SELECT
SCF_SELECT:
	testq %rdx, %rdx
	setne %dl
	movzbq %dl, %rax
	negq %rax
	movq %rax, %rcx
	andq %rdi, %rax
	notq %rcx
	andq %rsi, %rcx
	orq %rcx, %rax
	ret
