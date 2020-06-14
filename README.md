# FreeDVDBoot
PlayStation 2 DVD Player Exploit. This allows you to burn your own PlayStation 2 homebrew discs and play them on an unmodified console as seen in the [demo video](https://youtube.com/).

For technical details please refer to my [blog post](https://cturt.github.io/freedvdboot.html).

## Identify your DVD Player Version
Boot your PlayStation 2 without any disc inserted, and press Triangle.

For initial release only DVD Player version 3.10E is supported (as that's the console I have). Over time I hope other developers from the scene will add support for other DVD Player versions.

## Copy your homebrew
The exploit will attempt to boot `VIDEO_TS/VTS_02_0.IFO` as an `ELF`. The default file here is a [Tetris homebrew game by Viktor Gars](https://www.psx-place.com/resources/tetris.431/), but just replace this file with whatever you want.

For the initial release, I didn't reimplement a couple of functions used by the loader, so it requires that the ELF you load doesn't overwrite those functions I use (those are around `0x84000 - 0x85fff` and `0x250000 - 0x29ffff`). I will probably remove this limitation in the future, but all ELFs I could find were fine with it anyway.

You can run `readelf -l` to verify this. For example, the Tetris homebrew just uses `0x00100000 - 0x0017a940`:

	$ readelf -l VTS_02_0.IFO

	Elf file type is EXEC (Executable file)
	Entry point 0x104490
	There is 1 program header, starting at offset 52

	Program Headers:
	  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
	  LOAD           0x001000 0x00100000 0x00100000 0x72ef4 0x7a940 RWE 0x1000

	 Section to Segment mapping:
	  Segment Sections...
	   00     .text .ctors .dtors .rodata .data .jcr .sdata .sbss .bss

## Make an image
Once you've placed all the homebrew files you'd like into the directory, generate a UDF image of the directory. The easiest way is probably to install `genisoimage` and run the following (where `exploit.iso` is the output and `3.10E` is the directory containing `VIDEO_TS` and `AUDIO_TS`):

    genisoimage -udf -o exploit.iso 3.10E

## Test and burn
I would recommend you test in PCSX2 first, but since [PCSX2 doesn't support loading the DVD Player](https://github.com/PCSX2/pcsx2/issues/1981), you have to decrypt and repack it yourself, which can be a pain.
