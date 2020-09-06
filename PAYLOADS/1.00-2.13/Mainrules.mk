EE_CC = ee-gcc
EE_LD = ee-ld
EE_AS = ee-as
EE_OBJCOPY = ee-objcopy

IOP_CC = iop-gcc
IOP_LD = iop-ld
IOP_AS = iop-as
IOP_OBJCOPY = iop-objcopy
IOP_OBJDUMP = iop-objdump

IOP_SYMBOLS = -DREAD_SECTORS_210=$(IOP_READ_SECTORS_210) -DORIGINAL_RETURN_ADDRESS_210=$(IOP_ORIGINAL_RETURN_ADDRESS_210) -DRETURN_ADDRESS_LOCATION_210=$(IOP_RETURN_ADDRESS_LOCATION_210) \
	-DREAD_SECTORS_212=$(IOP_READ_SECTORS_212) -DORIGINAL_RETURN_ADDRESS_212=$(IOP_ORIGINAL_RETURN_ADDRESS_212) -DRETURN_ADDRESS_LOCATION_212=$(IOP_RETURN_ADDRESS_LOCATION_212) \
	-DREAD_SECTORS_213=$(IOP_READ_SECTORS_213) -DORIGINAL_RETURN_ADDRESS_213=$(IOP_ORIGINAL_RETURN_ADDRESS_213) -DRETURN_ADDRESS_LOCATION_213=$(IOP_RETURN_ADDRESS_LOCATION_213) \
	-DREAD_SECTORS_110=$(IOP_READ_SECTORS_110) -DORIGINAL_RETURN_ADDRESS_110=$(IOP_ORIGINAL_RETURN_ADDRESS_110) -DRETURN_ADDRESS_LOCATION_110=$(IOP_RETURN_ADDRESS_LOCATION_110)

IOP_CFLAGS = -O2 -G 0 -nostartfiles -nostdlib -ffreestanding -g $(IOP_SYMBOLS)

EE_CFLAGS = -O2 -G 0 -nostartfiles -nostdlib -ffreestanding -Wl,-z,max-page-size=0x1

IOP_STAGE1_SIZE_210_212 = `stat -c '%s' stage1_210_212.iop.bin`
IOP_STAGE1_SIZE_213 = `stat -c '%s' stage1_213.iop.bin`
IOP_PAYLOAD_SIZE = `stat -c '%s' ioppayload.iop.bin`

dvd.iso: dvd.base.iso stage1_210_212.iop.bin stage1_213.iop.bin ioppayload.iop.bin
	#genisoimage -udf -o dvd.iso udf/
	# @echo Insert 0x00000048 to offset 0x0818AC in dvd.iso
	# @echo Insert 0x00004000 to offset 0x0818B0 in dvd.iso
	# @echo Insert 0x000B7548 to offset 0x0818F4 in dvd.iso

	# For now it's easier to just use a base dvd rather than attempting to generate an image and patch it
	cp dvd.base.iso dvd.iso

	# Return address (2.10 - 2.13) 0x00818f4 = 530676
	printf $(STAGE1_LOAD_ADDRESS_STRING_210_212) | dd of=dvd.iso bs=1 seek=530676 count=4 conv=notrunc

	# Return address 1.10 (0x000818bc = 530620)
	printf $(STAGE1_LOAD_ADDRESS_STRING_110) | dd of=dvd.iso bs=1 seek=530620 count=4 conv=notrunc

	# Old toolchains don't support this option, so just copy byte-by-byte...
	# bs=4096 iflag=skip_bytes,count_bytes
	
	dd if=stage1_210_212.iop.bin of=dvd.iso bs=1 seek=$(STAGE1_ISO_210_212) count=$(IOP_STAGE1_SIZE_210_212) conv=notrunc
	dd if=stage1_213.iop.bin of=dvd.iso bs=1 seek=$(STAGE1_ISO_213) count=$(IOP_STAGE1_SIZE_213) conv=notrunc
	
	# 0x700000 = 7340032
	dd if=ioppayload.iop.bin of=dvd.iso bs=1 seek=7340032 count=$(IOP_PAYLOAD_SIZE) conv=notrunc

%.iop.bin: %.iop.elf
	$(IOP_OBJCOPY) -O binary $< $@

%.iop.o: %.iop.S
	$(IOP_AS) $< -o $@

stage1_210_212.iop.elf: stage1_210_212.iop.S ioppayload.iop.bin
	$(IOP_OBJDUMP) -t ioppayload.iop.elf | grep " _start"
	$(IOP_CC) $< -DENTRY=$(IOP_PAYLOAD_ENTRY) -DIOP_PAYLOAD_SIZE=$(IOP_PAYLOAD_SIZE) $(IOP_CFLAGS) -o $@

stage1_213.iop.elf: stage1_213.iop.S ioppayload.iop.bin
	$(IOP_OBJDUMP) -t ioppayload.iop.elf | grep " _start"
	$(IOP_CC) $< -DENTRY=$(IOP_PAYLOAD_ENTRY) -DIOP_PAYLOAD_SIZE=$(IOP_PAYLOAD_SIZE) $(IOP_CFLAGS) -o $@

%.iop.elf: %.iop.c eepayload.ee.bin
	$(IOP_CC) -Ttext=$(IOP_PAYLOAD_ADDRESS) -DLOAD_ELF_FROM_OFFSET=$(LOAD_ELF_FROM_OFFSET) ioppayload.iop.c $(IOP_CFLAGS) -o $@


%.ee.bin: %.ee.elf
	$(EE_OBJCOPY) -O binary $< $@ -Wl,-z,max-page-size=0x1

%.ee.o: %.ee.S
	$(EE_AS) $< -o $@

eepayload.ee.elf: eecrt0.ee.o syscalls.ee.o eepayload.ee.c
	$(EE_CC) -Ttext=$(EE_PAYLOAD_ADDRESS) $^ $(EE_CFLAGS) -o $@

clean:
	rm -rf *.elf *.bin *.o dvd.iso
