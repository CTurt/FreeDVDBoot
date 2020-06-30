echo "Building payload"

ee-gcc -Ttext=0x01FFF800 payload.c -o payload.elf -nostartfiles -nostdlib -ffreestanding -Os -Wl,-z,max-page-size=0x1 # 2048
ee-objcopy -O binary payload.elf payload.bin -Wl,-z,max-page-size=0x1

ENTRY=`ee-objdump -t payload.elf | grep " _start"`
echo $ENTRY

# Doesn't seem to work on MinGW toolchain, so set manually if you're using that:
#ENTRY=0x`grep -o "^\S*" <<< $ENTRY`
ENTRY=0x01fff9a8

echo $ENTRY
echo "Building crt0"

ee-gcc -Ttext=0x01500014 -DENTRY=$ENTRY crt0.S -o fullpayload.elf -nostartfiles -nostdlib -ffreestanding -Wl,-z,max-page-size=0x1
ee-objcopy -O binary fullpayload.elf fullpayload.bin -Wl,-z,max-page-size=0x1

echo "Done. Insert fullpayload.bin into VIDEO_TS.IFO at offset 0x2954"
