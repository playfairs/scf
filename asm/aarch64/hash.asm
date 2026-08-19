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
	rev x0, x0
	ret

.globl SCF_ROTL
SCF_ROTL:
	neg x1, x1
	ror x0, x0, x1
	ret

.globl SCF_ASM_SHA256
SCF_ASM_SHA256:
		stp x29, x30, [sp, #-16]!
		mov x29, sp
		stp x19, x20, [sp, #-16]!
		stp x21, x22, [sp, #-16]!
		sub sp, sp, #288
		mov x20, x0
		mov x21, x1
		mov x22, x2
	#ifdef SCF_DARWIN
		adrp x19, scf_sha256_k@PAGE
		add x19, x19, scf_sha256_k@PAGEOFF
	#else
		adrp x19, scf_sha256_k
		add x19, x19, :lo12:scf_sha256_k
	#endif
		cbz x22, .Lsha_done
.Lsha_block:
		ldp w8, w9, [x20]
		ldp w10, w11, [x20, #8]
		ldp w12, w13, [x20, #16]
		ldp w14, w15, [x20, #24]
		str w8, [sp, #256]
		str w9, [sp, #260]
		str w10, [sp, #264]
		str w11, [sp, #268]
		str w12, [sp, #272]
		str w13, [sp, #276]
		str w14, [sp, #280]
		str w15, [sp, #284]
		mov x0, #0
.Lsha_load:
		ldr w1, [x21, x0, lsl #2]
		rev w1, w1
		str w1, [sp, x0, lsl #2]
		add x0, x0, #1
		cmp x0, #16
		b.lo .Lsha_load
		mov x0, #16
.Lsha_expand:
		sub x1, x0, #15
		ldr w1, [sp, x1, lsl #2]
		ror w2, w1, #7
		ror w3, w1, #18
		eor w2, w2, w3
		lsr w1, w1, #3
		eor w2, w2, w1
		sub x1, x0, #2
		ldr w1, [sp, x1, lsl #2]
		ror w3, w1, #17
		ror w4, w1, #19
		eor w3, w3, w4
		lsr w1, w1, #10
		eor w3, w3, w1
		sub x1, x0, #16
		ldr w1, [sp, x1, lsl #2]
		add w1, w1, w2
		sub x2, x0, #7
		ldr w2, [sp, x2, lsl #2]
		add w1, w1, w2
		add w1, w1, w3
		str w1, [sp, x0, lsl #2]
		add x0, x0, #1
		cmp x0, #64
		b.lo .Lsha_expand

.macro SHA256_ROUND offset
		ldr w0, [sp, #\offset]
		ror w1, w12, #6
		ror w2, w12, #11
		eor w1, w1, w2
		ror w2, w12, #25
		eor w1, w1, w2
		and w2, w12, w13
		bic w3, w14, w12
		eor w2, w2, w3
		ldr w3, [x19, #\offset]
		add w0, w0, w15
		add w0, w0, w1
		add w0, w0, w2
		add w0, w0, w3
		ror w1, w8, #2
		ror w2, w8, #13
		eor w1, w1, w2
		ror w2, w8, #22
		eor w1, w1, w2
		and w2, w8, w9
		and w3, w8, w10
		eor w2, w2, w3
		and w3, w9, w10
		eor w2, w2, w3
		add w1, w1, w2
		mov w15, w14
		mov w14, w13
		mov w13, w12
		add w12, w11, w0
		mov w11, w10
		mov w10, w9
		mov w9, w8
		add w8, w0, w1
.endm
	SHA256_ROUND 0
	SHA256_ROUND 4
	SHA256_ROUND 8
	SHA256_ROUND 12
	SHA256_ROUND 16
	SHA256_ROUND 20
	SHA256_ROUND 24
	SHA256_ROUND 28
	SHA256_ROUND 32
	SHA256_ROUND 36
	SHA256_ROUND 40
	SHA256_ROUND 44
	SHA256_ROUND 48
	SHA256_ROUND 52
	SHA256_ROUND 56
	SHA256_ROUND 60
	SHA256_ROUND 64
	SHA256_ROUND 68
	SHA256_ROUND 72
	SHA256_ROUND 76
	SHA256_ROUND 80
	SHA256_ROUND 84
	SHA256_ROUND 88
	SHA256_ROUND 92
	SHA256_ROUND 96
	SHA256_ROUND 100
	SHA256_ROUND 104
	SHA256_ROUND 108
	SHA256_ROUND 112
	SHA256_ROUND 116
	SHA256_ROUND 120
	SHA256_ROUND 124
	SHA256_ROUND 128
	SHA256_ROUND 132
	SHA256_ROUND 136
	SHA256_ROUND 140
	SHA256_ROUND 144
	SHA256_ROUND 148
	SHA256_ROUND 152
	SHA256_ROUND 156
	SHA256_ROUND 160
	SHA256_ROUND 164
	SHA256_ROUND 168
	SHA256_ROUND 172
	SHA256_ROUND 176
	SHA256_ROUND 180
	SHA256_ROUND 184
	SHA256_ROUND 188
	SHA256_ROUND 192
	SHA256_ROUND 196
	SHA256_ROUND 200
	SHA256_ROUND 204
	SHA256_ROUND 208
	SHA256_ROUND 212
	SHA256_ROUND 216
	SHA256_ROUND 220
	SHA256_ROUND 224
	SHA256_ROUND 228
	SHA256_ROUND 232
	SHA256_ROUND 236
	SHA256_ROUND 240
	SHA256_ROUND 244
	SHA256_ROUND 248
	SHA256_ROUND 252
	ldr w0, [sp, #256]
	add w8, w8, w0
	ldr w0, [sp, #260]
	add w9, w9, w0
	ldr w0, [sp, #264]
	add w10, w10, w0
	ldr w0, [sp, #268]
	add w11, w11, w0
	ldr w0, [sp, #272]
	add w12, w12, w0
	ldr w0, [sp, #276]
	add w13, w13, w0
	ldr w0, [sp, #280]
	add w14, w14, w0
	ldr w0, [sp, #284]
	add w15, w15, w0
	stp w8, w9, [x20]
	stp w10, w11, [x20, #8]
	stp w12, w13, [x20, #16]
	stp w14, w15, [x20, #24]
	add x21, x21, #64
	subs x22, x22, #1
	b.ne .Lsha_block
.Lsha_done:
	add sp, sp, #288
	ldp x21, x22, [sp], #16
	ldp x19, x20, [sp], #16
	ldp x29, x30, [sp], #16
	ret

#ifdef SCF_DARWIN
.section __TEXT,__const
#else
.section .rodata
#endif
.p2align 2
scf_sha256_k:
	.word 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5
	.word 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5
	.word 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3
	.word 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174
	.word 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc
	.word 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da
	.word 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7
	.word 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967
	.word 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13
	.word 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85
	.word 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3
	.word 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070
	.word 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5
	.word 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3
	.word 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208
	.word 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
