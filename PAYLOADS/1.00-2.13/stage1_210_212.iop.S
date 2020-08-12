# ElReino & CTurt 2020

flushIcache = 0x00002f40
flushDcache = 0x0003044

#flushDcacheWrapper = 0x0057f1c

iop_payload_address = 0xa00fd000

.section .text

.global _start
_start:
	move $fp, $sp # We need to reset $fp as it gets trashed by memcpy

check_210:
	la $v1, 0x27bdffc8 # addiu $sp, $sp, -0x38
	la $v0, READ_SECTORS_210
	lw $t0, 0($v0)
	beq $t0, $v1, read_iop_payload

check_212:
	la $v0, READ_SECTORS_212

read_iop_payload:
	la $a0, (IOP_PAYLOAD_SIZE / 0x800) + 1 # count
	la $a1, 0x700000 / 0x800 # sector
	la $a2, iop_payload_address # destination
	jal $v0
	
	#jal flushIcache
	#jal flushDcache
	
	#jal ENTRY
	la $v0, ENTRY
	jalr $v0

check_210_again:
	la $v1, 0x27bdffc8 # addiu $sp, $sp, -0x38
	la $v0, READ_SECTORS_210
	lw $v0, 0($v0)
	la $a0, RETURN_ADDRESS_LOCATION_210
	la $ra, ORIGINAL_RETURN_ADDRESS_210
	beq $v0, $v1, return

check_212_again:
	la $a0, RETURN_ADDRESS_LOCATION_212
	la $ra, ORIGINAL_RETURN_ADDRESS_212

return:
	# Return gracefully back to original return address
	sw $ra, 0($a0)
	la $v0, 0
	jr $ra
