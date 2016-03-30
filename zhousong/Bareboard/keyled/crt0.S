@**********************************************************
@File:     crt0.S
@Function: entry to C code via this file
@Coding based on WDS source code 2016-03-21
@**********************************************************

.text
.global _start
_start:
	ldr	r0, =0x53000000	@ Watchdog reg addr
	mov	r1, #0x0
	str	r1, [r0]	@ write '0'; disable Watchdog
	ldr	sp, =1024*4	@ setup stack (4K)
	bl	main		@ call main function in C
halt_loop:
	b	halt_loop
