#include "easypdk/pdk.h"
#include <stdint.h>

#include "pdkspi.h"
#include "pdkspi_pins.h"

void pdkspi_init(void)
{
  //set SPI NCS, CLK, OUT as outputs
  SPI_PORTC |= (1<<SPI_NCS_PIN) | (1<<SPI_CLK_PIN) | (1<<SPI_OUT_PIN);
  //set NCS = 1, OUT = 1
  SPI_PORT |= (1<<SPI_NCS_PIN) | (1<<SPI_OUT_PIN);
}

uint8_t pdkspi_sendreceive(uint8_t s)
{
  __asm
     mov a, #8                                 ; loop 8 times
1$:
     set0.io _ASMS(SPI_PORT), #(SPI_OUT_PIN)   ; SPI-OUT = 0
     t0sn _pdkspi_sendreceive_PARM_1, #7       ; s.7==0 ?
     set1.io _ASMS(SPI_PORT), #(SPI_OUT_PIN)   ; SPI-OUT = 1
     set1.io _ASMS(SPI_PORT), #(SPI_CLK_PIN)   ; SPI-CLK = 1
     sl _pdkspi_sendreceive_PARM_1             ; s<<=1
     t0sn.io _ASMS(SPI_PORT), #(SPI_IN_PIN)    ; SPI-IN==0 ?
     set1 _pdkspi_sendreceive_PARM_1, #0       ; s.0=1
     set0.io _ASMS(SPI_PORT), #(SPI_CLK_PIN)   ; SPI-CLK = 0
     dzsn a                                    ; loop--
     goto 1$                                   ; loop again if>0
  __endasm;
  return s;
}

/*
uint8_t pdkspi_sendreceive(uint8_t s)
{
  for( uint8_t p=8; p>0; p-- )
  {
    SPI_PORT &= ~(1<<SPI_OUT_PIN);             //first always clear DAT (shorter than than if/else)
    if( s&0x80 )                               //then check if set is required
      SPI_PORT |= (1<<SPI_OUT_PIN);            //set DAT

    SPI_PORT |= (1<<SPI_CLK_PIN);              //set CLK
    s <<= 1;
    if( SPI_PORT&(1<<SPI_IN_PIN) )
      s |= 1;

    SPI_PORT &= ~(1<<SPI_CLK_PIN);             //clear CLK
  }
  return s;
}
*/
