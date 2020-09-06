# ElReino and CTurt 2020

# Since GCC does something strange, we can't write syscall thunks directly in C
# as GCC adds move $v1, $v0 directly after jr $ra, effectively trashing $v0.
# I don't know why this happens, but I do know enough about GCC that this
# approach will most probably be easier. But feel free to try fixing it.

.global GetThreadId
GetThreadId:
        la $v1, 0x2f
        syscall 0x2f
        jr $ra

.global ChangeThreadPriority
ChangeThreadPriority:
        la $v1, 0x29
        syscall 0x29
        jr $ra

.global CancelWakeupThread
CancelWakeupThread:
        la $v1, 0x35
        syscall 0x35
        jr $ra

.global TerminateThread
TerminateThread:
        la $v1, 0x25
        syscall 0x25
        jr $ra

.global DeleteThread
DeleteThread:
        la $v1, 0x21
        syscall 0x21
        jr $ra

.global SifSetReg
SifSetReg:
       la $v1, 0x79
       syscall 0x79
       jr $ra

.global SifGetReg
SifGetReg:
	la $v1, 0x7a
	syscall 0x7a
	jr $ra

.global ExecPS2
ExecPS2:
	la $v1, 0x07
	syscall 0x07 # BTW why do we put the number here also?

# Not a syscall, but it might as well be.

.global SifWriteBackDCache
SifWriteBackDCache:
        lui     $25, 0xffff
        ori     $25, $25, 0xffc0
        blez    $5, last
        addu    $10, $4, $5
        and     $8, $4, $25
        addiu   $10, $10, -1
        and     $9, $10, $25
        subu    $10, $9, $8
        srl     $11, $10, 0x6
        addiu   $11, $11, 1
        andi    $9, $11, 0x7
        beqz    $9, eight
        srl     $10, $11, 0x3
loop1:
        sync
        cache   0x18, 0($8)
        sync
        addiu   $9, $9, -1
        nop
        bgtz    $9, loop1
        addiu   $8, $8, 64
eight:
        beqz    $10, last
loop8:
        addiu   $10, $10, -1
        sync
        cache   0x18, 0($8)
        sync
        cache   0x18, 64($8)
        sync
        cache   0x18, 128($8)
        sync
        cache   0x18, 192($8)
        sync
        cache   0x18, 256($8)
        sync
        cache   0x18, 320($8)
        sync
        cache   0x18, 384($8)
        sync
        cache   0x18, 448($8)
        sync
        bgtz    $10, loop8
        addiu   $8, $8, 512
last:
        jr      $31
        nop
