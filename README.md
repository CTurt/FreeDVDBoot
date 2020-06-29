# FreeDVDBoot
PlayStation 2 DVD Player Exploit. This allows you to burn your own PlayStation 2 homebrew discs and play them on an unmodified console as seen in the [demo video](https://www.youtube.com/watch?v=ez0y-hz3VuM). With uLaunchELF as the initial program, users can include multiple homebrew programs on the same disc.

For technical details please refer to my [blog post](https://cturt.github.io/freedvdboot.html).

## Step 1: Identify your DVD Player Version
Boot your PlayStation 2 without any disc inserted, and press Triangle to identify which DVD Player version your console has. Still early in terms of support for different versions.

Language is also confirmed to affect the exploit (https://www.youtube.com/watch?v=zelVQcD7HCY), so please set your PS2 language in the system configuration to match supported configuration (it should be possible to port to other languages in the future, but I'm prioritising different firmware versions instead of different lanauges to start with, since language can be changed).

So far I only tested 3.10 on E and U on hardware, and 3.11 isn't confirmed working on hardware yet (J with English language).

Currently only support:

- 3.10 (E or U - with English language set in settings)
- 3.11 (J - with English language set in settings)

Check back here later for more support. Hopefully over time other developers from the scene will also contribute support for additional DVD Player versions. Don't bother trying on a not supported region/firmware configuration, it won't work...

Pre-built ISO files for supported DVD Players containing just uLaunchELF are provided in this repository for ease of use (which can be used to boot homebrew over USB storage), such as `3.10EU.iso`. If you intend to make your own image containing additional homebrew / modified initial loader, please read on.

## Step 2: Copy your homebrew
Once you've identified your console's DVD Player version, copy all of the homebrew you would like to include on the disc into that directory (EG: `3.10EU/`).

## Step 3: Make an image
Once you've placed all the homebrew files you'd like into the directory, generate a UDF image of the directory. The easiest way is probably to install `genisoimage` and run the following (where `exploit.iso` is the output and `3.10EU` is the directory containing `VIDEO_TS` and any homebrew):

    genisoimage -udf -o exploit.iso 3.10EU

## Step 4: Test and burn
I would recommend you test in PCSX2 first, but since [PCSX2 doesn't support loading the DVD Player](https://github.com/PCSX2/pcsx2/issues/1981), you have to decrypt and repack it yourself, which is beyond the scope of this README. With that said, if you aren't touching anything in `VIDEO_TS`, there shouldn't really be any reason for the exploit to fail.

You should use DVD-R, and finalize the disc as burning option.

## OPTIONAL: Replace the initial program
I've included uLaunchELF recompiled with [DVD support](https://github.com/ps2dev/ps2sdk/pull/130) as the default initial program. It presents a menu which allows you to select any of the homebrew programs you chose to include on the disc (and also allows booting from USB).

Alternatively, if you would rather just boot into a single homebrew application, the initial program the exploit attempts to boot is located at `VIDEO_TS/VTS_02_0.IFO`, replace it with your desired `ELF` file, with the below caveat that compatibility might be lower than if you booted a program through uLaunchELF:

For the initial release, I didn't bother to reimplement a couple of functions used by the loader, so it requires that the ELF you load doesn't overwrite those functions I use (those are around `0x84000 - 0x85fff` and `0x250000 - 0x29ffff`). I will probably remove this limitation in the future, but all ELFs I could find were fine with this limitation.

You can run `readelf -l` to verify your executable satisfies this requirement. For example, this Tetris homebrew just uses `0x00100000 - 0x0017a940`:

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

## DEVELOPMENT: Replacing the loader payload
The default payload will boot `VIDEO_TS/VTS_02_0.IFO` as an ELF file, but tweaks might be desired to improve compatibility, or maybe changing the behaviour to boot `BOOT.ELF` instead for instance.

If you wish to update the loader payload, run `build.sh` inside `PAYLOAD` directory, and copy the output `fullpayload.bin` to `VIDEO_TS/VIDEO_TS.IFO` at offset `0x2bb4` (for 3.10E).
