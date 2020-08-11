STAGE1_LOAD_ADDRESS = 0xa00b7548
STAGE1_LOAD_ADDRESS_STRING = '\x48\x75\x0b\xa0'

IOP_READ_SECTORS = 0xb260c
IOP_ORIGINAL_RETURN_ADDRESS = 0xb3630
IOP_RETURN_ADDRESS_LOCATION = 0x1f62ac


#IOP_PAYLOAD_ENTRY = `$(IOP_OBJDUMP) -t ioppayload.iop.elf | grep " _start"`
IOP_PAYLOAD_ENTRY = 0xa00fd178 # Set this manually for now.

IOP_PAYLOAD_ADDRESS = 0xa00fd000

EE_PAYLOAD_ADDRESS = 0x01fff800

#isoinfo -l -i dvd.iso | grep "BOOT.ELF"
#var=`isoinfo -l -i dvd.iso | grep "BOOT.ELF" | grep -o -P "[0-9]*? -"`
# LOAD_ELF_FROM_OFFSET = 
LOAD_ELF_FROM_OFFSET = 0x5BB000 # Set this manually for now

include Mainrules.mk
