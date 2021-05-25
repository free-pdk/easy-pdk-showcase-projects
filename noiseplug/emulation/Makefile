all:
	$(MAKE) -C ../ PFS154
	gcc -Wall -O3 `sdl2-config --cflags` -I. microemu_pfs154.c noiseplugsim.c -o noiseplugsim `sdl2-config --libs`

clean:
	rm noiseplugsim

.PHONY: all clean
