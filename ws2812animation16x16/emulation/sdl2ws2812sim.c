#include <stdint.h>

// select emulation target
#define PFS173
//#define PFS154
//#define PMS150C

// rename main function to pdk_main and setup macros to emulate SDCC compatibility
#define __SDCC_pdk13
#define __SDCC_pdk14
#define __SDCC_pdk15
#define __sfr   volatile uint8_t
#define __sfr16 volatile uint16_t
#define __at(x)
#pragma push_macro("__asm__")
#undef __asm__
#define __asm__(x)
#pragma push_macro("main")
#undef main
#define main pdk_main
// include the original source
#include "../ws2812animation16x16.c"
#pragma pop_macro("main")
#pragma pop_macro("__asm__")

//// EMULATION

#include <stdio.h>
#include <SDL.h>

#define WS2812_X 16
#define WS2812_Y 16

static uint32_t framebuf[WS2812_X * WS2812_Y];
static uint32_t emuws2812bytepos = 0;
static uint64_t emuws2812pticks = 0;
static SDL_Window   *window;
static SDL_Renderer *renderer;
static SDL_Texture  *texture;

//helper function to update emulated ws2812 matrix
static void _emu_ws2812_update(void) {
  //>50 usec no data ==> data is activated (we use a higher value since host os might not be realtime enough)
  if( emuws2812bytepos && ((SDL_GetPerformanceCounter()-emuws2812pticks)*1000000/SDL_GetPerformanceFrequency()>5000) ) {
    SDL_UpdateTexture(texture, NULL, framebuf, WS2812_X * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    emuws2812bytepos = 0;
  }
  SDL_Event event;
  if( SDL_PollEvent(&event) &&  (SDL_QUIT==event.type) )
    exit(0);
}

//emulation of IC version of ws2812 init function
void pdkws2812_init(void) {
  emuws2812bytepos = 0;
}

//emulation of IC version of ws2812 send function (will store pixel in frame buffer)
void pdkws2812_send_8bit(uint8_t v) {
  _emu_ws2812_update();
  uint32_t pixpos = emuws2812bytepos/3;
  uint32_t pixrgb = emuws2812bytepos%3;
  uint32_t pixpos_y = pixpos / WS2812_X;
  uint32_t pixpos_x = pixpos % WS2812_X;
  //'un-zig-zag' the matrix
  if( 0==(pixpos_y & 1) )
    pixpos_x = WS2812_X - 1 - pixpos_x;
  switch( pixrgb ) {
    case 0: framebuf[pixpos_y*WS2812_X + pixpos_x] =  ((uint32_t)v)<<8;  break;
    case 1: framebuf[pixpos_y*WS2812_X + pixpos_x] |= ((uint32_t)v)<<16; break;
    case 2: framebuf[pixpos_y*WS2812_X + pixpos_x] |= ((uint32_t)v)<<0;  break;
  }
  emuws2812bytepos++;
  emuws2812pticks=SDL_GetPerformanceCounter();
}

//emulation of IC version of pdkdelay (splits delay in smaller parts and calls emu_pdkws2812_update function)
void pdkdelay(uint32_t msec) {
  for( ;msec>0; msec-- ) {
    _emu_ws2812_update();
    SDL_Delay(1);
  }
}

//glue code to start SDL2 in a window with framebuffer, call the emulated render function and show result
int main(int argc, char* argv[]) {
  if( (SDL_Init(SDL_INIT_VIDEO)<0) ||
      !(window = SDL_CreateWindow("WS2812MATRIX", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WS2812_X*40, WS2812_Y*40, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)) ||
      !(renderer = SDL_CreateRenderer(window, -1, 0)) ||
      !(texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WS2812_X, WS2812_Y))
    )
  {
    fprintf(stderr, "ERROR: SDL: %s\n", SDL_GetError());
    return -1;
  }
  atexit(SDL_Quit);

  _sdcc_external_startup();                         //simulate call to _sdcc_external_startup()
  pdk_main();                                       //simulate jump to main()

  return 0;
}
