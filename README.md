# chip4504
CHIP4504 was a hobby project of mine to extend the CHIP-8 CPU to accomodate more instructions as well as implement the existing SuperCHIP-8 instruction set. This project was a result of that. Unlike a normal emulator, this one is an operating system in itself, written in x86 assembly. Although it is nothing very fancy, the kernel uses x86 BIOS interrupt calls and the OS runs entirely in real mode. It has not been tested on real hardware, and only on QEMU.

## Features

* 64K of memory access with segmentation
* Easy to navigate UI
* Integrated text file view for viewing READMEs etc.
* FAT12 file system to load ROMs from floppies
* Supports CHIP-8, and SuperCHIP-8 instructions (in addition to the new 4504 instructions)

## Current Issues

* Slow emulation
* Problems with user input
* No assembler for the new instruction set

## Manual

The manual is incomplete for now. However, the CPU, memory addressing modes, and the instruction set is documented.

## Running

Simply run the "qemu.bat" file under Windows. (Ensure that you have the qemu-system-i386 binaries and have set up the PATH variable accordingly).
Otherwise, you can use the emulator of your choice, VirtualBox or VMWare.

## Building

You will need the SmallerC compiler, which you can find here: https://github.com/alexfru/SmallerC and the Netwide Assembler,
from here: http://www.nasm.us/ . In addition you'll need a floppy disk program to browse and edit the virtual copy of floppies
that will be used. (An existing copy called 'chip4504.ima' is present in this repository.

## Contributing

Report issues to the "Issues" section under this github repository. Other than that, uh, just clone? I wouldn't mind
pull requests if they improve or add up on the current system.

## Special Thanks

Special thanks to MikeOS and their development team. Quite a few underlying parts of the code are borrowed [more like shamelessly copied :)],
from there.

## Screenshots

![alt tag](http://i.imgur.com/OECGIVQ.png)
![alt tag](http://i.imgur.com/ZBagWfk.png)
![alt tag](http://i.imgur.com/PerMPOL.png)
![alt tag](http://i.imgur.com/qx4uYxF.png)
![alt tag](http://i.imgur.com/QbtKT3d.png)
![alt tag](http://i.imgur.com/wrHxYJi.png)
![alt tag](http://i.imgur.com/nO5Eevv.png)
