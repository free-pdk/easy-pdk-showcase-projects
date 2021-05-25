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


Wiring:
=======

- PADAUK GND    ==> SPEAKER
- PADAUK PA.3   ==> SPEAKER

