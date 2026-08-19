.text
.p2align 2
#ifdef SCF_DARWIN
#define SCF_BYTESWAP _scf_asm_byteswap64
#define SCF_ROTL _scf_asm_rotl64
#define SCF_ASM_SHA256 _scf_asm_sha256_compress
#else
#define SCF_BYTESWAP scf_asm_byteswap64
#define SCF_ROTL scf_asm_rotl64
#define SCF_ASM_SHA256 scf_asm_sha256_compress
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

.globl SCF_ASM_SHA256
SCF_ASM_SHA256:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	subq $352, %rsp
	movq %rdi, -328(%rbp)
	movq %rsi, -336(%rbp)
	movq %rdx, -344(%rbp)
	leaq scf_sha256_k(%rip), %rbx
	testq %rdx, %rdx
	je .Lsha_done
.Lsha_block:
	movq -328(%rbp), %rdi
	movq -336(%rbp), %rsi
	movl 0(%rdi), %r8d
	movl 4(%rdi), %r9d
	movl 8(%rdi), %r10d
	movl 12(%rdi), %r11d
	movl 16(%rdi), %r12d
	movl 20(%rdi), %r13d
	movl 24(%rdi), %r14d
	movl 28(%rdi), %r15d
	movl %r8d, -320(%rbp)
	movl %r9d, -316(%rbp)
	movl %r10d, -312(%rbp)
	movl %r11d, -308(%rbp)
	movl %r12d, -304(%rbp)
	movl %r13d, -300(%rbp)
	movl %r14d, -296(%rbp)
	movl %r15d, -292(%rbp)
	movl $0, %ecx
.Lsha_load:
	movl (%rsi,%rcx,4), %eax
	bswapl %eax
	movl %eax, -256(%rbp,%rcx,4)
	incq %rcx
	cmpq $16, %rcx
	jb .Lsha_load
	xorl %ecx, %ecx
.Lsha_round:
	cmpq $16, %rcx
	jb .Lsha_ready
	movq %rcx, %rax
	subq $15, %rax
	movl -256(%rbp,%rax,4), %eax
	movl %eax, %edx
	roll $25, %edx
	movl %eax, %esi
	roll $14, %esi
	xorl %esi, %edx
	shrl $3, %eax
	xorl %eax, %edx
	movq %rcx, %rax
	subq $2, %rax
	movl -256(%rbp,%rax,4), %eax
	movl %eax, %edi
	roll $15, %edi
	movl %eax, %esi
	roll $13, %esi
	xorl %esi, %edi
	shrl $10, %eax
	xorl %eax, %edi
	movq %rcx, %rax
	subq $16, %rax
	movl -256(%rbp,%rax,4), %eax
	addl %edx, %eax
	movq %rcx, %rdx
	subq $7, %rdx
	addl -256(%rbp,%rdx,4), %eax
	addl %edi, %eax
	movl %eax, -256(%rbp,%rcx,4)
.Lsha_ready:
	movl %r12d, %eax
	roll $26, %eax
	movl %r12d, %edx
	roll $21, %edx
	xorl %edx, %eax
	movl %r12d, %edx
	roll $7, %edx
	xorl %edx, %eax
	movl %r12d, %edx
	andl %r13d, %edx
	movl %r12d, %esi
	notl %esi
	andl %r14d, %esi
	xorl %esi, %edx
	addl %r15d, %eax
	addl %edx, %eax
	addl (%rbx,%rcx,4), %eax
	addl -256(%rbp,%rcx,4), %eax
	movl %r8d, %edx
	roll $30, %edx
	movl %r8d, %esi
	roll $19, %esi
	xorl %esi, %edx
	movl %r8d, %esi
	roll $10, %esi
	xorl %esi, %edx
	movl %r8d, %edi
	andl %r9d, %edi
	movl %r8d, %esi
	andl %r10d, %esi
	xorl %esi, %edi
	movl %r9d, %esi
	andl %r10d, %esi
	xorl %esi, %edi
	addl %edi, %edx
	addl %edx, %eax
	movl %r14d, %r15d
	movl %r13d, %r14d
	movl %r12d, %r13d
	addl %eax, %r11d
	movl %r11d, %r12d
	movl %r10d, %r11d
	movl %r9d, %r10d
	movl %r8d, %r9d
	addl %edx, %eax
	movl %eax, %r8d
	incq %rcx
	cmpq $64, %rcx
	jb .Lsha_round
	addl -320(%rbp), %r8d
	addl -316(%rbp), %r9d
	addl -312(%rbp), %r10d
	addl -308(%rbp), %r11d
	addl -304(%rbp), %r12d
	addl -300(%rbp), %r13d
	addl -296(%rbp), %r14d
	addl -292(%rbp), %r15d
	movq -328(%rbp), %rdi
	movl %r8d, 0(%rdi)
	movl %r9d, 4(%rdi)
	movl %r10d, 8(%rdi)
	movl %r11d, 12(%rdi)
	movl %r12d, 16(%rdi)
	movl %r13d, 20(%rdi)
	movl %r14d, 24(%rdi)
	movl %r15d, 28(%rdi)
	movq -336(%rbp), %rsi
	addq $64, %rsi
	movq %rsi, -336(%rbp)
	decq -344(%rbp)
	jnz .Lsha_block
.Lsha_done:
	addq $352, %rsp
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %rbx
	popq %rbp
	ret

.section .rodata
.p2align 2
scf_sha256_k:
	.long 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5
	.long 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5
	.long 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3
	.long 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174
	.long 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc
	.long 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da
	.long 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7
	.long 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967
	.long 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13
	.long 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85
	.long 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3
	.long 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070
	.long 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5
	.long 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3
	.long 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208
	.long 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
