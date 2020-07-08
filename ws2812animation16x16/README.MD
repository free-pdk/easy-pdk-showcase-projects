### WS2812 16x16 matrix animation ###

- original project: https://github.com/joshgerdes/arduino-led-matrix

- emulation folder contains simulation on host pc (uses original source in emulation)

- packbitmaps folder contains simple RLE compressor to reduce the bitmaps to fit in much smaller PDK MCUs

![DEMO](.img/ws2812matrixdemo.png?raw=true "DEMO")


Build:
======

- download and install latest SDCC version: http://sdcc.sourceforge.net/snap.php

run:
$ make 


Write to IC:
============

PFS173:
$ easypdkprog --icname=PFS173 write build/ws2812animation16x16.PFS173.ihx

PFS172:
$ easypdkprog --icname=PFS172 write build/ws2812animation16x16.PFS172.ihx

PFS154:
$ easypdkprog --icname=PFS154 write build/ws2812animation16x16.PFS154.ihx

PMS150C:
$ easypdkprog --icname=PMS150C write build/ws2812animation16x16.PMS150C.ihx


Wiring:
=======

ws2812matrix16x16 +5V ==> PADAUK VCC
ws2812matrix16x16 GND ==> PADAUK GND
ws2812matrix16x16 DIN ==> PADAUK PA.6


WS2812-RGB-16x16-Matrix
=======================

You can find a WS2812B 16x16 matrix on ebay for less than $15.00 (search for "WS2812b 16x16")
