.text

.globl scf_test_asm
.type scf_test_asm, @function

scf_test_asm:
    mov %rdi, %rax
    add $1, %rax
    ret