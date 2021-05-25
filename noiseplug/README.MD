### NOISEPLUG sound player ###

- original project: https://github.com/dop3j0e/noiseplug

- emulation folder contains simulation on host pc (emulates a PFS154 and runs the compiled code)


Build:
======

- download and install latest SDCC version: http://sdcc.sourceforge.net/snap.php

run:
$ make 


Write to IC:
============

PMS150C:
$ easypdkprog --icname=PMS150C write build/noiseplug.PMS150C.ihx

PFS154:
$ easypdkprog --icname=PFS154 write build/noiseplug.PFS154.ihx

PFS172:
$ easypdkprog --icname=PFS172 write build/noiseplug.PFS172.ihx

PFS173:
$ easypdkprog --icname=PFS173 write build/noiseplug.PFS173.ihx


Wiring:
=======

- PADAUK GND    ==> SPEAKER
- PADAUK PA.3   ==> SPEAKER

