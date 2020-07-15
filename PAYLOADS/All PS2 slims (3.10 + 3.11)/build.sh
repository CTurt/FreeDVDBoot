echo "Building payload"

ee-gcc -Ttext=0x01FFF800 payload.c -o payload.elf -nostartfiles -nostdlib -ffreestanding -Os -Wl,-z,max-page-size=0x1 # 2048
ee-objcopy -O binary payload.elf payload.bin -Wl,-z,max-page-size=0x1

ENTRY=`ee-objdump -t payload.elf | grep " _start"`
echo $ENTRY

# Doesn't seem to work on MinGW toolchain, so set manually if you're using that:
#ENTRY=0x`grep -o "^\S*" <<< $ENTRY`
ENTRY=0x01fff99c

echo $ENTRY

echo "Building crt0 (3.10)"

ee-gcc -Ttext=0x01500014 -DENTRY=$ENTRY -DGETBUFFERINTERNAL=0x2986a0 crt0.S -o crt0_3.10.elf -nostartfiles -nostdlib -ffreestanding -Wl,-z,max-page-size=0x1
ee-objcopy -O binary crt0_3.10.elf crt0_3.10.bin -Wl,-z,max-page-size=0x1

echo "Building crt0 (3.11)"

ee-gcc -Ttext=0x01500014 -DENTRY=$ENTRY -DGETBUFFERINTERNAL=0x2952f0 crt0.S -o crt0_3.11.elf -nostartfiles -nostdlib -ffreestanding -Wl,-z,max-page-size=0x1
ee-objcopy -O binary crt0_3.11.elf crt0_3.11.bin -Wl,-z,max-page-size=0x1

echo "Done."
echo "Insert crt0_3.10.bin into VIDEO_TS.IFO at offset 0x2bb4"
echo "Insert crt0_3.11.bin into VIDEO_TS.IFO at offset 0x2954"
echo "Insert payload.bin into VIDEO_TS.IFO at offset 0x3000"
