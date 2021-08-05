# ZZombie

## The Challenge

Even in 2021, AmigaOS, the operating system for Commodore Amiga computers originally made in the late 80s and early 90s, is still a commercial non-copyleft product licensed/owned/sold by two companies (different versions by each), so you cannot legally make a useful work environment for testing Amiga software or bringing up Workbench, the desktop that is built into ROM but needs a library call from software to spring to life.

My challenge was to find a legal way to distribute a standard USB stick image that can be auto-booted by the MNT ZZ9000 multifunction expansion card (graphics/network/ram/usb storage). ZZombie is a collection of commands, libraries and tools from only either open source projects ([AROS](https://github.com/aros-development-team/AROS)), public domain or shareware packages from Aminet that allow you to do some useful things with an Amiga that has only a 3.1 ROM (possibly also earlier ROM versions, still untested). Useful tasks are:

- File management with Workbench
- Shell (NewCLI)
- Going online via ZZ9000 Ethernet (AmiTCP 3)
- Download files (wget)
- Mount ADF disk images

This way we can hopefully provide a known-good software environment for testing ZZ9000 and excluding any software configuration problems that are due to individual setups (these vary greatly among Amiga users).

Moreover, this is also just a fun challenge and feedback is very appreciated.

If you think that any license of any included program is violated by this distribution, please get in touch and I will immediately remove it--especially if you are the author.

*Please note that this is not intended as a base for a production system!* It is merely a pre-installation/last resort/insane technical demo environment for when you really don't care about the quality of the components, just that it works somehow. It is held together by chewing gum and wire. Get the real AmigaOS, P96 etc. instead!

## Usage

TODO: Provide a disk image

Currently, Vim is the bundled editor and replacement for Ed, so you need to know how to operate Vim.

Everything is prepared to go online with AmiTCP, but you need to adjust `AmiTCP:bin/startnet` and change `192.168.1.222` to your desired IP address and `192.168.1.1` to the correct gateway. DHCP is not supported.

To go online, launch a new CLI (Amiga+E, `NewCLI`) and enter `online`. Another CLI window with a harmless S2-related error pops up, close it. You can then use the original CLI to use AmiTCP commands like `ping` to verify that you are really online.

If everything works, you can use `wget <url>` to download things, i.e. to `RAM:` and extract archives with `lha`. For example: `wget http://aminet.net/disk/misc/ImageMount.lha`.

Please note that the included open-source version of AmiTCP is not exactly fast, but it is enough to get you bootstrapped.

## The Parts: Minimal Version

### Commands

Most C: commands come from [AROS m68k](https://github.com/aros-development-team/AROS). Some commands and all AROS GUI tools crash with original ROMs, so these working ones were carefully selected:

- Assign (custom modified build, see below)
- Avail
- Copy
- Date
- Delete
- Dir
- DiskChange
- Eval
- Execute
- Filenote
- Info
- Install
- Join
- LoadWB
- MakeDir
- Mount
- Protect
- Relabel
- Rename
- Run
- Touch
- Type
- Wait
- Which

The license for these tools is the [AROS APL](https://github.com/aros-development-team/AROS/blob/master/LICENSE) (based on Mozilla Public License).

The `Assign` command from AROS crashes, so I heavily edited the source and compiled it with bebbo-gcc and made it work. The modified source is in the `custom` folder.

Additional commands and tools:

- lha https://aminet.net/package/util/arc/lha License: "Permissive License"
- VersionWB (`VersionWB LIKECVER` replaces Version via an Alias in `Startup-Sequence`) http://aminet.net/package/util/sys/VersionWB License: "Gift-Ware"
- RTPatch (see asl.library below)
- ToolX (replaces IconX) http://aminet.net/package/util/sys/toolx24 License: "Freeware"

### Libraries

Most libraries come from [AROS m68k](https://github.com/aros-development-team/AROS):

- amigaguide.library (Untested)
- commodities.library
- datatypes.library
- diskfont.library
- iffparse.library
- locale.library
- mathieeedoubbas.library
- mathieeedoubtrans.library
- mathieeesingbas.library
- mathieeesingtrans.library
- mathtrans.library
- rexxsupport.library
- rexxsyslib.library
- version.library

Other libraries:

- As we don't have asl.library, it is stubbed by:
  - [NewReqLibs](http://aminet.net/package/util/libs/NewReqLibs18) License: Freeware
  - and replaced by [ReqTools](http://aminet.net/package/util/libs/ReqToolsUsr.lha) License: Freeware or Shareware, depending on context. ("ReqTools is Copyright © Nico François and Magnus Holmgren")

### Classes, Datatypes

- colorwheel.gadget (RTG capable replacement) http://aminet.net/package/dev/gui/ColorWheel (from AROS)
- TODO: gradientslider.gadget

### DOSDrivers

- PIPE (from AROS)

### Prefs

- None.

### Useful Software

These are preinstalled but also the original archives are included:

- AmiTCP 3.0 https://aminet.net/package/comm/net/AmiTCP-bin-30b2 License: GPL
- Ixemul 48 (POSIX libraries required for wget) https://aminet.net/package/util/libs/ixemul-48.0 License: BSD?
- Wget 1.8.2 (Downloader tool) http://aminet.net/package/comm/tcp/wget-1.8.2 License: GPL
- ZZ9000 Network Driver

### Fonts

- None yet, Topaz is in ROM.

### Locale

- None.

### S:Startup-Sequence

- Custom written.
