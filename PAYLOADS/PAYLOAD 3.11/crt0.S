.set noreorder # If we're writing assembly, why would we want this?

.section .text.startup

.equ getBufferInternal, 0x2952f0
.equ payload, (0x2000000 - 0x800) # End of RAM

.global _start
_start:
	#la $a0, 0x7f
	#la $v1, 0x01
	#syscall 0x01 # ResetEE

	#la $a0, relo
	la $a0, load
	la $a1, 0
	la $a2, 0
	la $a3, 0

.global ExecPS2
ExecPS2:
	la $v1, 7
	syscall 7 # ExecPS2

load:
	la $a0, 0
	la $a1, 0 # 0 = VIDEO_TS.IFO, 1 = VTS_01_0.IFO
	la $a2, 0x3000 / 0x800 # lba offset in file
	la $a3, payload # Destination
	la $t0, 0x800 / 0x800 # Count
	la $t1, 0
	la $v0, getBufferInternal
	jalr $v0
	nop

#relo:
	# Relocate payload to end of RAM
#	la $a0, (0x2000000 - 1024)
#	la $a1, payload
#	la $a2, 2048
#	la $v0, memcpy
#	jalr $v0

boot:
	la $v1, 0x64; la $a0, 0; syscall 0x64 # FlushCache data writeback
	la $v1, 0x64; la $a0, 2; syscall 0x64 # FlushCache instruction invalidate

	# Point stack to end of scratchpad RAM
	la $sp, 0x70004000

	# Execute from relocated place
	la $v0, ENTRY
	j $v0
	nop

.space (_start + 0x2fe2 - 0x2954) - .
fpIndex:
	.byte 0xfa
	.byte 0xdc

.space (_start + 0x3000 - 0x2954) - .
.incbin "payload.bin"
