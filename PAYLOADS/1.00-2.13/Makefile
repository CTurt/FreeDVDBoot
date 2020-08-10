#STAGE1_LOAD_ADDRESS = 0x1f62b0
#STAGE1_LOAD_ADDRESS = 0xa01f62b0 # repacked
#STAGE1_LOAD_ADDRESS = 0xA01F30B0 # (0xa0000000 + 0x01f62b0 + 0x1F3058 - 0x1f6258) # hardware

# (0xb7548 + 0x5c700 - 0xb1000) = 0x62C48
STAGE1_LOAD_ADDRESS = 0xa0062C48 # hardware

EE_CC = ee-gcc
EE_LD = ee-ld
EE_AS = ee-as
EE_OBJCOPY = ee-objcopy

IOP_CC = iop-gcc
IOP_LD = iop-ld
IOP_AS = iop-as
IOP_OBJCOPY = iop-objcopy
IOP_OBJDUMP = iop-objdump

IOP_CFLAGS = -O2 -G 0 -nostartfiles -nostdlib -ffreestanding -g

EE_CFLAGS = -O2 -G 0 -nostartfiles -nostdlib -ffreestanding -Wl,-z,max-page-size=0x1

#IOP_PAYLOAD_ENTRY = `$(IOP_OBJDUMP) -t ioppayload.iop.elf | grep " _start"`
#IOP_PAYLOAD_ENTRY = 0xa0460178 # Set this manually for now.
IOP_PAYLOAD_ENTRY = 0xa00fd178 # Set this manually for now.

IOP_STAGE1_SIZE = `stat -c '%s' stage1.iop.bin`
IOP_PAYLOAD_SIZE = `stat -c '%s' ioppayload.iop.bin`
#IOP_PAYLOAD_ADDRESS = 0x460000
#IOP_PAYLOAD_ADDRESS = 0xa0460000
IOP_PAYLOAD_ADDRESS = 0xa00fd000

EE_PAYLOAD_ADDRESS = 0x01fff800

#isoinfo -l -i dvd.iso | grep "BOOT.ELF"
#var=`isoinfo -l -i dvd.iso | grep "BOOT.ELF" | grep -o -P "[0-9]*? -"`
# LOAD_ELF_FROM_OFFSET = 
LOAD_ELF_FROM_OFFSET = 0x5BB000 # Set this manually for now

all: dvd.iso

dvd.iso: dvd.base.iso stage1.iop.bin ioppayload.iop.bin
	cp dvd.base.iso dvd.iso
	
	#genisoimage -udf -o dvd.iso udf/
	# @echo Insert 0x00000048 to offset 0x0818AC in dvd.iso
	# @echo Insert 0x00004000 to offset 0x0818B0 in dvd.iso
	# @echo Insert 0x000B7548 to offset 0x0818F4 in dvd.iso

	# bs=4096 iflag=skip_bytes,count_bytes
	
	# 0x820f8 = 532728
	dd if=stage1.iop.bin of=dvd.iso bs=1 seek=532728 count=$(IOP_STAGE1_SIZE) conv=notrunc
	# 0x700000 = 7340032
	dd if=ioppayload.iop.bin of=dvd.iso bs=1 seek=7340032 count=$(IOP_PAYLOAD_SIZE) conv=notrunc

%.iop.bin: %.iop.elf
	$(IOP_OBJCOPY) -O binary $< $@

%.iop.o: %.iop.S
	$(IOP_AS) $< -o $@

stage1.iop.elf: stage1.iop.S ioppayload.iop.bin
	$(IOP_OBJDUMP) -t ioppayload.iop.elf | grep " _start"
	$(IOP_CC) -Ttext=$(STAGE1_LOAD_ADDRESS) $< -DENTRY=$(IOP_PAYLOAD_ENTRY) -DIOP_PAYLOAD_SIZE=$(IOP_PAYLOAD_SIZE) $(IOP_CFLAGS) -o $@

ioppayload.iop.elf: ioppayload.iop.c eepayload.ee.bin
	$(IOP_CC) -Ttext=$(IOP_PAYLOAD_ADDRESS) -DLOAD_ELF_FROM_OFFSET=$(LOAD_ELF_FROM_OFFSET) ioppayload.iop.c $(IOP_CFLAGS) -o $@


%.ee.bin: %.ee.elf
	$(EE_OBJCOPY) -O binary $< $@ -Wl,-z,max-page-size=0x1

%.ee.o: %.ee.S
	$(EE_AS) $< -o $@

eepayload.ee.elf: eecrt0.ee.o syscalls.ee.o eepayload.ee.c
	$(EE_CC) -Ttext=$(EE_PAYLOAD_ADDRESS) $^ $(EE_CFLAGS) -o $@

clean:
	rm -rf *.elf *.bin *.o dvd.iso
