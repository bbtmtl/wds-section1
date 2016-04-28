@*************************************************************
@File:     head.S
@Function: Setup SDRAM, cp code to SDRAM and jump to there
@          then exacute them
@Coding based on WDS source code 2016-04-08 
@*************************************************************

.text
.global _start
_start:

	ldr	sp,	=1024*4				@ setup stack 
	bl	disable_watch_dog		@ disable WATCHDOG -> init.c
	bl	memsetup				@ setup store reg -> init.c
	bl	nand_init				@ init Nand Flash -> init.c

	ldr r0,		=0x30000000		@ Target addr=start addr of SDRAM
	mov r1,		#4096			@ Source addr; main.c code is here when link
	mov r2,		#2048			@ cp length=2K bytes for maine.c code
	bl  nand_read				@ call c function nand_read

	ldr sp,		=0x34000000		@ setup stack
	ldr lr,		=halt_loop		@ set return addr
	ldr pc, 	=main			@ jump to main to exacute
								@ b and bl can noly jump 32M around
halt_loop:
	b	halt_loop
