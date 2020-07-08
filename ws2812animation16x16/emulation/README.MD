## WS2812MATRIX EMULATION - FREEPDK ##

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
$ ./sdl2ws2812sim

Windows (use MinGW32 shell):
$ ./sdl2ws2812sim.exe
