# ZZombie

## The Challenge

Even in 2021, AmigaOS, the operating system for Commodore Amiga computers originally made in the late 80s and early 90s, is still a commercial non-copyleft product licensed/owned/sold by two companies (different versions by each), so you cannot legally make a useful work environment for testing Amiga software or bringing up Workbench, the desktop that is built into ROM but needs a library call from software to spring to life.

My challenge was to find a legal way to distribute a standard USB stick image that can be auto-booted by the MNT ZZ9000 multifunction expansion card (graphics/network/ram/usb storage). ZZombie is a collection of commands, libraries and tools from only either open source projects ([AROS](https://github.com/aros-development-team/AROS)), public domain or shareware packages from Aminet that allow you to do some useful things with an Amiga that has only a 3.1 ROM (possibly also earlier ROM versions, still untested). Useful tasks are:

- File management with Workbench
- Shell (NewCLI)
- Going online via ZZ9000 Ethernet (Demo version of Miami)
- Download files (wget)
- Rudimentary web (AWeb APL)
- Mount ADF disk images
- Play WHDLoad games (if you have them)
- Limited RTG (with old public version of Picasso96)

This way we can hopefully provide a known-good software environment for testing ZZ9000 and excluding any software configuration problems that are due to individual setups (these vary greatly among Amiga users).

Moreover, this is also just a fun challenge and feedback is very appreciated.

If you think that any license of any included program is violated by this distribution, please get in touch and I will immediately remove it--especially if you are the author.

*Please note that this is not intended as a base for a production system!* It is merely a pre-installation/last resort/insane technical demo environment for when you really don't care about the quality of the components, just that it works somehow. It is held together by chewing gum and wire. Get the real AmigaOS, P96 etc. instead!

## The Parts

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

The `Assign` command from AROS crashes, so I heavily edited the source and compiled it with bebbo-gcc and made it work. The modified source is in the `custom` folder.

Additional commands and tools:

- FastIPrefs (replaces IPrefs) http://aminet.net/package/util/boot/FastIPrefs4035
- Installer (MUI replacement) http://aminet.net/package/util/sys/Installer
- MuchMore (replaces More) http://aminet.net/package/text/show/muchmore46
- OS 2.0+ MultiView replacement http://aminet.net/package/util/sys/2b_mv_os2
- lha https://aminet.net/package/util/arc/lha
- VersionWB (`VersionWB LIKECVER` replaces Version via an Alias in `Startup-Sequence`) http://aminet.net/package/util/sys/VersionWB
- RTPatch (see asl.library below)
- ToolX (replaces IconX) http://aminet.net/package/util/sys/toolx24
- SetPatch 43.6b http://aminet.net/package/util/boot/SetPatch_43.6b
- Redit (text editor, Aliased as replacement for Ed) https://aminet.net/package/text/edit/

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

- As we don't have asl.library, it is stubbed by [NewReqLibs](http://aminet.net/package/util/libs/NewReqLibs18) and replaced by [ReqTools](http://aminet.net/package/util/libs/ReqToolsUsr.lha)
- FileID.library http://aminet.net/package/util/libs/FIDLib80
- notifyintuition.library http://aminet.net/package/util/libs/NotInt24

### Classes, Datatypes

- picture.datatype replacement is installed by Picasso96 if we sneakily create an empty dummy file before
- colorwheel.gadget (RTG capable replacement) http://aminet.net/package/dev/gui/ColorWheel
- TODO: gradientslider.gadget

### DOSDrivers

- PIPE from AROS

### Prefs

- WBPattern from FastIPrefs http://aminet.net/package/util/boot/FastIPrefs4035
- ScreenMode: MUIScrMode http://aminet.net/package/util/wb/MUIScrMode1_5

### Useful Software

These are preinstalled but also the original archives are included:

- MUI 3.8, the Magical User Interface (Shareware) https://aminet.net/package/util/libs/mui38usr
- WHDLoad http://aminet.net/package/dev/misc/WHDLoad_usr
- MiamiDx Demo Version (TCP/IP Stack) https://aminet.net/package/comm/tcp/MiamiDx10cmain and http://aminet.net/package/comm/tcp/MiamiDx10c-MUI
- AWeb APL Lite 3.5.09 (Basic HTML browser) http://aminet.net/package/comm/www/AWeb-3.5.09-68000
- Ixemul 48 (POSIX libraries required for wget) https://aminet.net/package/util/libs/ixemul-48.0
- Wget 1.8.2 (Downloader tool) http://aminet.net/package/comm/tcp/wget-1.8.2
- Picasso96 2.0 (Obsolete version, but can do the basics) https://aminet.net/package/driver/video/Picasso96
- SnoopDos https://aminet.net/package/util/moni/SnoopDos
- ZZ9000 Drivers

### Fonts

- None yet, Topaz is in ROM.

### Locale

- None.

### S:Startup-Sequence

- Custom written.

## Unsolved Problems: Help Wanted

1. Missing monitor drivers (`PAL` and `NTSC` files normally in `Devs:Monitors`) make it hard to use MUIScrMode. Unlike the original ScreenMode Prefs, it doesn't seem to pick up the built-in PAL and NTSC screenmodes.
2. FastIPrefs has weird behavior. It seems to auto-hide the menu bar of Workbench when Backdrop is on?!
3. AmigaGuide files don't seem to be rendered by the MultiView replacement
