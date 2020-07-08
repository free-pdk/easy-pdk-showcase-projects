//adopted from: https://github.com/joshgerdes/arduino-led-matrix
#include <stdint.h>
#include "easypdk/pdk.h"

#include "pdkws2812.h"
#include "pdkdelay.h"

#include "animations.h"

void drawAnimation(const uint8_t* pal, const uint8_t* const* images, const uint8_t imgcount, const uint16_t delaymsec, uint8_t repeat) {
  do {                                                                          //repeat loop
    for (uint8_t i=0; i<imgcount; i++) {                                        //iterate over images in animation
      uint8_t* picptr = (uint8_t*)images[i];
      for( uint16_t out=16*16; out>0; ) {                                       //decompress and output one image (16x16)
        uint8_t dat = *picptr++;                                                //get next byte from compressed image
        uint8_t len = (dat>>3)+1;                                               //extract run length
        dat = (dat&0x07)*3;                                                     //extract palette index
        for( ;len>0; len-- ) {                                                  //output the the same pixel 'len' times
          pdkws2812_send_8bit(pal[dat+1]);                                      //send green (palette is RGB)
          pdkws2812_send_8bit(pal[dat+0]);                                      //send red
          pdkws2812_send_8bit(pal[dat+2]);                                      //send blue
          out--;                                                                //count pixel as done
        }
      }
      pdkdelay(delaymsec);                                                      //wait to show image
    }
  } while( repeat-- >0 );
}

unsigned char _sdcc_external_startup(void) {
  EASY_PDK_INIT_SYSCLOCK_8MHZ();                                                //use 8MHz sysclock
  EASY_PDK_CALIBRATE_IHRC(8000000,5000);                                        //tune SYSCLK to 8MHz @ 5.000V
  return 0;                                                                     //perform normal initialization
}

void main(void) {
  pdkws2812_init();                                                             //initialize WS2812 output

  for(;;) {                                                                     //endless loop, show different animations
    for( uint8_t i=0; i<sizeof(ws2812animations)/sizeof(ANIM); i++ ) {          //iterate over animations
      drawAnimation( ws2812animations[i].pal,
                     ws2812animations[i].images,
                     ws2812animations[i].imgcount,
                     ws2812animations[i].delaymsec,
                     ws2812animations[i].repeat );
    }
  }
}
