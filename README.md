# FreeDVDBoot
PlayStation 2 DVD Player Exploit. This allows you to burn your own PlayStation 2 homebrew discs and play them on an unmodified console as seen in the [demo video](https://www.youtube.com/watch?v=ez0y-hz3VuM). With uLaunchELF as the initial program, users can include multiple homebrew programs on the same disc.

For technical details please refer to my [blog post](https://cturt.github.io/freedvdboot.html).

## Basic setup
Using prebuilt ISOs in this repo ([PREBUILT ISOs](https://github.com/CTurt/FreeDVDBoot/tree/master/PREBUILT%20ISOs) folder).

### Step 1: Identify your DVD Player Version
Boot your PlayStation 2 without any disc inserted, and press Triangle to identify which DVD Player version your console has. Still early in terms of support for different versions, check back here later for more support. Hopefully over time other developers from the scene will also contribute support for additional DVD Player versions.

**Currently only support:**

- 3.04 (tested only region M in emulator so far, but guess all regions EUMACDGJ will work - with English language set in settings) - please ping me and will update this page when confirmed working on hardware.
- 3.10 (all regions EUMACDGJ - with English language set in settings) - confirmed working on hardware by CTurt, and others. [Only seems to work with English language](https://www.youtube.com/watch?v=zelVQcD7HCY).
- 3.11 (all regions EUMACDGJ) - confirmed working on hardware by [MrMario2011](https://twitter.com/MrMario2011/status/1277586569738813440), and others. ([Only seems to work with English language](https://twitter.com/kood_infothief/status/1277600247024238592)).

UPDATE: Experimental hybrid ISO for both 3.10 and 3.11 support merged into one now available, burn `PREBUILT ISOs/hybrid 3.10 and 3.11.iso` and set language to English. Confirmed [working on 3.11](https://twitter.com/TheWizWiki/status/1277670129355161601).

**Please don't bother trying on a non-supported firmware/language configuration, it won't work...**

### Step 2: Burn
Pre-built ISO files for supported DVD Players containing just uLaunchELF are provided in this repository for ease of use (which can be used to boot homebrew over a USB mass storage device). For example, if your DVD Player version is 3.10E, you would want to burn `PREBUILT ISOs/3.10 only - all regions - English lang.iso`.

You should use DVD-R with low burning speed (others may work, but they put more strain on PS2 laser), and make sure to finalise the disc as burning option. Otherwise, you might run into issues reading the disc.

## Troubleshooting - please read if the above didn't work
Disc doesn't spin on slim console - press the lid down hard to ensure the sensors detect that the lid is closed.

PS2 says "unable to read disc" - this doesn't seem to be a problem with the exploit, but just that your DVD laser might not work, or at least can't read the disc you burned. Please try a regular DVD video first, or DVD game (not CD game) to ensure one of those works. In fact, playing a real DVD video can recalibrate your laser and cause burned discs to work as [commented here](https://github.com/CTurt/FreeDVDBoot/issues/27). If that works, make sure you are finalising your disc when burning, use a low write speed, and I recommend using DVD-R instead of any other type of DVD as those put more strain on the laser.

PS2 enters black screen - if your PS2 DVD laser is really worn out, or you are using something difficult to read like DVD+RW burned on high speed, with a dusty disc, it might take some time before uLaunchELF actually starts. Please try waiting 3 minutes or so, per [this comment](https://github.com/CTurt/FreeDVDBoot/issues/3#issuecomment-651337741).

PS2 detects the disc as "PlayStation 2 disc" instead of "DVD Video" in the browser - Your PS2 has a modchip which is incorrectly preventing you from booting the DVD player. You do not need this exploit for a console with a modchip.

Try unplugging your controller, and plugging it back in. Apparently [that solved the issue for this user](https://github.com/CTurt/FreeDVDBoot/issues/103).

Try removing all memory cards. Apparently [that solved the issue for this user](https://github.com/CTurt/FreeDVDBoot/issues/3#issuecomment-651970564).

Also try cleaning the disc to remove dust, and try verifying the burn on PC.

If all else fails, you can try using the prebuilt ISO specifically for your version instead of the hybrid image. I have had reports of people claiming that [the hybrid image doesn't work, whilst 3.11 does for them](https://github.com/CTurt/FreeDVDBoot/issues/13). I have also heard from some people that [the hybrid works for them, whilst the 3.10 image doesn't work for them](https://github.com/CTurt/FreeDVDBoot/issues/48). I have also had people claiming that [the exact same image both works and doesn't work for them](https://github.com/CTurt/FreeDVDBoot/issues/10#issuecomment-651739012), so who knows how accurate these reports are...

## Custom disc setup
If you intend to make your own image containing additional homebrew / modified initial loader, please read on. Step 1 is the same; identify your firmware version.

### Step 2: Copy your homebrew
Once you've identified your console's DVD Player version, copy all of the homebrew you would like to include on the disc into that directory in the `Filesystem` (EG: `Filesystem/3.10EU/`).

### Step 3: Make an image
Once you've placed all the homebrew files you'd like into the directory, generate a UDF image of the directory (or ISO/UDF hybrid also works)

On Windows, you can use a GUI like ImgBurn to make an disc image. It will give a warning that `VIDEO_TS.BUP` is missing, but just click continue anyway (PS2 doesn't require this file).

On Linux the easiest way is probably to use `genisoimage` as it comes pre-installed on many Linux distributions like Ubuntu. Run the following on terminal (where `exploit.iso` is the output and `Filesystem/3.10` is the directory containing `VIDEO_TS` and any homebrew):

    genisoimage -udf -o exploit.iso Filesystems/3.10

### Step 4: Test and burn
I would recommend you test in PCSX2 first, but since [PCSX2 doesn't support loading the DVD Player](https://github.com/PCSX2/pcsx2/issues/1981), you have to decrypt and repack it yourself, which is beyond the scope of this README. With that said, if you aren't touching anything in `VIDEO_TS`, there shouldn't really be any reason for the exploit to fail.

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

## PORTING:
Please read my technical writeup, to understand how the exploit works. I've also provided some notes about porting in the `porting notes.txt` file.
