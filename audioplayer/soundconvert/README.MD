### CONVERT SOUND TO RAW 31250Hz MONO

#download song
$ curl -o sound.mp3 https://files.freemusicarchive.org/storage-freemusicarchive-org/music/ccCommunity/Nathaniel_Wyvern/2019_releases_collection/Nathaniel_Wyvern_-_Infiltrators.mp3

#convert .mp3 to raw audio using SOX
$ sox sound.mp3 --channels 1 --bits 8 --encoding unsigned-integer --rate 31250 sound.raw

#write the sound.raw to SPI flash
! future: easypdkprog can be used to write external SPI flash by implementing a "flashrom" driver !

#truncate the audio - for testing only -
$ truncate -s 512KB sound.raw

#convert the raw audio to C include file - for simulation only -
$ xxd -c 32 -i sound.raw >sound_raw.h


required tools: CURL,SOX,XXD
============================
MacOS:
$ brew install curl sox xxd

Linux:
$ apt install curl sox xxd

Windows (MSYS2-Shell):
$ pacman -S curl sox xxd

music credits:
==============
Nathaniel_Wyvern, 2019 collection : 01. Infiltrators (04:53) - CC BY 4.0
https://www.freemusicarchive.org/music/Nathaniel_Wyvern

