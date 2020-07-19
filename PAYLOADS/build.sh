echo "Building payload"

ee-gcc -Ttext=0x01FFF800 payload.c -o payload.elf -nostartfiles -nostdlib -ffreestanding -Os -Wl,-z,max-page-size=0x1 # 2048
ee-objcopy -O binary payload.elf payload.bin -Wl,-z,max-page-size=0x1

ENTRY=`ee-objdump -t payload.elf | grep " _start"`
echo $ENTRY

# Doesn't seem to work on MinGW toolchain, so set manually if you're using that:
#ENTRY=0x`grep -o "^\S*" <<< $ENTRY`
ENTRY=0x01fff99c

echo $ENTRY

echo "Building crt0 (3.03)"
ee-gcc -Ttext=0x015FFF34 -DENTRY=$ENTRY -DGETBUFFERINTERNAL=0x262360 crt0.S -o crt0_3.03.elf -nostartfiles -nostdlib -ffreestanding -Wl,-z,max-page-size=0x1
ee-objcopy -O binary crt0_3.03.elf crt0_3.03.bin -Wl,-z,max-page-size=0x1

echo "Building crt0 (3.04M)"
ee-gcc -Ttext=0x01800180 -DENTRY=$ENTRY -DGETBUFFERINTERNAL=0x261548 crt0.S -o crt0_3.04M.elf -nostartfiles -nostdlib -ffreestanding -Wl,-z,max-page-size=0x1
ee-objcopy -O binary crt0_3.04M.elf crt0_3.04M.bin -Wl,-z,max-page-size=0x1

echo "Building jump for 3.04J"
ee-gcc -Ttext=0x012811E4 -DJUMP=0x01281340 jump.S -o jump.elf -nostartfiles -nostdlib -ffreestanding -Wl,-z,max-page-size=0x1
ee-objcopy -O binary jump.elf jump.bin -Wl,-z,max-page-size=0x1

echo "Building crt0 (3.04J)"
ee-gcc -Ttext=0x01281340 -DENTRY=$ENTRY -DGETBUFFERINTERNAL=0x261560 crt0.S -o crt0_3.04J.elf -nostartfiles -nostdlib -ffreestanding -Wl,-z,max-page-size=0x1
ee-objcopy -O binary crt0_3.04J.elf crt0_3.04J.bin -Wl,-z,max-page-size=0x1

echo "Building crt0 (3.10)"
ee-gcc -Ttext=0x01500014 -DENTRY=$ENTRY -DGETBUFFERINTERNAL=0x2986a0 crt0.S -o crt0_3.10.elf -nostartfiles -nostdlib -ffreestanding -Wl,-z,max-page-size=0x1
ee-objcopy -O binary crt0_3.10.elf crt0_3.10.bin -Wl,-z,max-page-size=0x1

echo "Building crt0 (3.11)"
ee-gcc -Ttext=0x01500014 -DENTRY=$ENTRY -DGETBUFFERINTERNAL=0x2952f0 crt0.S -o crt0_3.11.elf -nostartfiles -nostdlib -ffreestanding -Wl,-z,max-page-size=0x1
ee-objcopy -O binary crt0_3.11.elf crt0_3.11.bin -Wl,-z,max-page-size=0x1

echo "Done."

echo "For the All Slims image:"
echo "Insert crt0_3.03.bin into VIDEO_TS.IFO at offset 0x0e8c"
echo "Insert jump.bin into VIDEO_TS.IFO at offset 0x2724"
echo "Insert crt0_3.04J.bin into VIDEO_TS.IFO at offset 0x2880"
echo "Insert crt0_3.10.bin into VIDEO_TS.IFO at offset 0x2bb4"
echo "Insert crt0_3.11.bin into VIDEO_TS.IFO at offset 0x2954"
echo "Insert payload.bin into VIDEO_TS.IFO at offset 0x3000"

echo "For 3.04M only image:"
echo "Insert fullpayload.bin at 0x2d00, and payload.bin at 0x3000"
