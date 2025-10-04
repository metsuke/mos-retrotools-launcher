VZEM
VZ200/VZ300 Emulator for windows
Written by Guy Thomason
Email: intertek00@netspace.net.au
Build 20200223


INTRODUCTION

VZEM is an emulator for the VZ200/VZ300 computers, also known as the Laser 200/
Laser 310 (Europe), Texet TX8000 (UK), and Salora Fellow (Finland).

The VZ200 was manufactured by Video Technology in Hong Kong in 1983. It was 
distributed in Australia by Dick Smith electronics, and was a popular first 
computer for many users due to its low cost. In 1983 the VZ200 sold for $199, 
the Commodore 64 by comparison sold for around $800.


USAGE

VZEM requires the following files to exist in the same directory as vzem.exe

* vzrom.v20		VZ200 rom image
* vzdos.rom		Dos rom image (if disk emulation is required)
* vzem.cfg		Configuration file

The directory should be write enabled to allow changes to the .cfg file

When vzem.exe is first run it will look for vzem.cfg. If this is not found VZEM
will create it with basic default configuration options.

From within the emulator, the hardware configuration can be changed from the
“Options” menu.  To make this configuration permanent, select the “Save Config”
option.

VZEM accepts the following command-line arguments
-f	load snapshot file
-d	mount disk image 

Examples:
vzem.exe -f galaxon.vz		load galaxon snapshot on startup
vzem.exe -d extbasic.dsk	mount extbasic.dsk on startup

Creating a .bat file with command line options will allow the user to start
vzem and run a snapshot automatically from windows explorer


LOADING/SAVING files

Programs can be loaded or saved by:

* Snapshots (.VZ files)
* Cassette emulation using Windows Wav files
* Disk emulation using a disk image

To save a BASIC program to a snapshot image, select File - Save VZ
To save a BASIC program to a Wav file, follow the following steps
1. In the emulator, type CSAVE “filename” but do not press enter
2. Select File - Cassette - Record
3. Enter the name of the PC file you wish to create and click SAVE
4. Press enter from the emulator

You should hear the cassette tones as the file is being saved. 

5. Once the save is complete and the tones have stopped, select
       File - Cassette - Stop  
      
It is very important to “stop” the recording just as you would if you 
physically saved a program to tape. If you do not stop, the file length is
indeterminate and the file will not be a valid wav file.

To save a program to disk, select File – Mount Disk. Select an existing disk
image, or type the name of a new file. If using an existing disk image, the
program can be saved with the SAVE “filename” command. When creating a new disk
image, the disk must first be formatted with the INIT command.
(See disk commands)


EXTENDED GRAPHICS

The “Super VZ Graphics” hardware modification is emulated. This provides 
support for all the graphics modes the video chip is capable of (Motorola 6847)

To active the extended graphics modes, type the following from the emulator:

GM0	OUT 32,0	64x64		Color			1024 bytes
GM1	OUT 32,4	128x64		Monochrome		1024 bytes
GM2	OUT 32,8	128x64		Color			2048 bytes
GM3	OUT 32,12	128x96		Monochrome		1536 bytes
GM4	OUT 32,16	128x96		Color			3072 bytes
GM5	OUT 32,20	128x192		Monochrome		3072 bytes
GM6	OUT 32,24	128x192		Color			6144 bytes
GM7	OUT 32,28	256x192		Monochrome		6144 bytes

Then activate the mode using the MODE(1) command or POKE 26624,8.

The VZ200 memory map can only access 2k video ram. For graphics modes that 
occupy more than 2k (eg GM7), bank switching is used. 

The byte written to port 32 is decoded via the following bits

xxxmmmbb

Where xxx is not used, mmm is the graphics mode, and bb is the bank. So to 
activate GM7 bank 0, the output byte would be 0011100 binary or 28 decimal.
To activate GM7 bank 1, the output byte would be 0011101 binary or 29 decimal. 

For whatever bank is active, reads/writes to the 2k video memory at 7000h will
affect the screen. For GM7, pixel rows 0-63 are active for bank 0, 64-127 for 
bank 1 and 128-191 for bank 2. 

For further information on extended vz graphics, read the following articles 
(available from VZAlive)

VZ Super Graphics by Joe Leon
Ultra Graphics Adapter by Matthew Sorrell.  

Support has been added for the German Graphics Mod. 
From the main menu, select options -> extended graphics -> german.
The German modification only supports GM7, the 256x192 monochrome mode. To
enable the mode, set bit 4 of address 30779, eg POKE 30779,8
Then activate the hi-res screen by either the MODE(1) command or POKE 26624,8.

The bank switching is controlled by port 222, eg

OUT 222,0	selects bank 0
OUT 222,1	selects bank 1
OUT 222,2	selects bank 2
OUT 222,3	selects bank 3


CREDITS

Thanks to the following people who directly supplied me with information, or
that I have stolen ideas and code from:

Marat Fayzullin		Z80 Emulation library. His MSX emulator motivated me to 
			develop a VZ200 emulator   	 

Juergen Buchmueller	Disk emulation routines and other bits and pieces

Richard Wilson		Screen timing information for use in split screen modes


