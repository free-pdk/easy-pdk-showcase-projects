### Audio player using external flash ###

- good quality audio player (32150Hz @8bit mono) using PWM output

- reads data from external flash and outputs as PWM

- example schematic with external flash and audio amplifier (see schematic folder)

- emulation folder contains simulation on host pc (uses original source in emulation)


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


