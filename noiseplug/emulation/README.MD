## NOISEPLUG EMULATION - FREEPDK ##

This emulation contains a tiny emulator for the PFS154 (just the ISA, no peripherals)

required packages: SDL2, SDCC
=============================

Download and install latest SDCC version: http://sdcc.sourceforge.net/snap.php

MacOS:
$ brew install sdl2 

Linux:
$ apt install libsdl2-dev

Windows (MSYS2-Shell):
$ pacman -S mingw32/mingw-w64-i686-SDL2


compile:
========
MacOS / Linux:
$ make

Windows (use MinGW32 shell):
$ make


run:
====
MacOS / Linux:
$ ./noiseplugsim

Windows (use MinGW32 shell):
$ ./noiseplugsim.exe
