# ElReino & CTurt 2020

.section .text.startup

.global _start
_start:
	# Point stack to end of scratchpad RAM
	#la $sp, 0x70004000

.global main
	#la $v1, 0x01
	#la $a0, 0x7f
	#syscall 0x01 # ResetEE
	
	la $a0, main
	la $a1, 0
	la $a2, 0
	la $a3, 0

	jr $a0

# Don't use on phat PS2... completely broken syscall
#ExecPS2:
#	la $v1, 0x07
#	syscall 0x07 # ExecPS2
