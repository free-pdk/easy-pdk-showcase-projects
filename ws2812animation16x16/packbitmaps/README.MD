## SIMPLE RLE PACKER FOR WS2812 MATRIX BITMAPS ##

- original images from: https://github.com/joshgerdes/arduino-led-matrix

- compression uses palettes with 8 colors

- compresses 41 pictures (1024 byte each) = 41984 bytes to 2674 bytes (compression factor 15.6)

- output of c code on stdout

- compile: gcc packbitmaps.c -o packbitmaps

