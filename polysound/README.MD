### Polyphonic sound player emulating the 4 NES sound and playing a famous tune ###

- original project: https://bisqwit.iki.fi/jutut/kuvat/programming_examples/pic/

- emulation folder contains simulation on host pc (uses original source in emulation)


Build:
======

- download and install latest SDCC version: http://sdcc.sourceforge.net/snap.php

run:
$ make 


Write to IC:
============

PFS173:
$ easypdkprog --icname=PFS173 write build/polysound.PFS173.ihx

PFS172:
$ easypdkprog --icname=PFS172 write build/polysound.PFS172.ihx

PFS154:
$ easypdkprog --icname=PFS154 write build/polysound.PFS154.ihx

PMS150C:
$ easypdkprog --icname=PMS150C write build/polysound.PMS150C.ihx


Wiring:
=======

PADAUK GND    ==> SPEAKER
PADAUK PA.6   ==> SPEAKER

