## POLYSOUND EMULATION - FREEPDK ##

required package: SDL2
======================
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
$ ./polysoundsim

Windows (use MinGW32 shell):
$ ./polysoundsim.exe
