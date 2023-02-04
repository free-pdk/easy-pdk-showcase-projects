#include "easypdk/pdk.h"
#include "audioplayer.h"
#include "pdkdelay.h"
#include "pdkspi.h"

#include <stdint.h>

#ifndef SEND_SAMPLE
#define SEND_SAMPLE(n)  {TM2B = n;}
#endif

volatile uint8_t next_sample;

void interrupt(void) __interrupt(0)
{
  if(INTRQ & INTRQ_T16)                       //interrupt request for timer16?
  {
    SEND_SAMPLE(next_sample);                 //setup new PWM value (next_sample)
    next_sample = 0xFF;                       //mark next_sample consumed (0xFF is not allowed sample byte)
    INTRQ &= ~INTRQ_T16;                      //clear this interrupt request
  }
}

unsigned char __sdcc_external_startup(void)
{
  EASY_PDK_INIT_SYSCLOCK_4MHZ();              //use 4MHz so we can use 3.0V VDD
  EASY_PDK_CALIBRATE_IHRC(4000000,3000);      //tune SYSCLK = 4MHz @ 3.000V
  return 0;                                   //perform normal initialization
}

void main(void)
{
  pdkspi_init();

  //// PWM setup
  //set PWM as output
  PWM_PORTC |= (1<<PWM_PIN);
  TM2C = TM2C_CLK_IHRC | TM2C_OUT_PA3 | TM2C_MODE_PWM;
  TM2S = TM2S_PWM_RES_8BIT | TM2S_PRESCALE_NONE | TM2S_SCALE_NONE; // IHRC(16MHz) /1 /1 /256(8bit) = 62500 Hz
  TM2B = 0x80;                                //set pwm output to silence (50%)

  //// AMP setup
  AMP_NSHDN_PORTC |= (1<<AMP_NSHDN_PIN);      //set AMP NSHDN as output
  AMP_NSHDN_PORT &= ~(1<<AMP_NSHDN_PIN);      //enable shutdown of amp

  pdkdelay(2000);                             //wait 2 sec before playing

  //// SPI external flash start read (0x03 = continuous read, address 0x000000)
  pdkspi_select();                            //select flash chip
  pdkspi_sendreceive(0x03);                   //SPI flash command 0x03 (read)
  pdkspi_sendreceive(0x00);                   //addr0
  pdkspi_sendreceive(0x00);                   //addr1
  pdkspi_sendreceive(0x00);                   //addr2

  //// TIMER16 + interrupt setup
  T16M   = T16_CLK_IHRC | T16_CLK_DIV1 | T16_INTSRC_8BIT; //IHRC, /1, BIT8 ==> /512 = 31250 Hz
  INTEN  = INTEN_T16;                         //enable T16 interrupt
  INTEGS = 0;
  INTRQ  = 0;

  next_sample = pdkspi_sendreceive(0xff);     //fetch first sample and put in buffer

  //// START AMP+PLAYBACK
  AMP_NSHDN_PORT |= (1<<AMP_NSHDN_PIN);       //disable shutdown of amp
  pdkdelay(100);                              //wait 100 msec to turn on amp softly

  uint8_t t = pdkspi_sendreceive(0xff);       //prefetch 2nd sample to temp

  __engint();                                 //global enable interrupt

  for(;t!=0xff;)                              //loop until we reach end of data (0xff is not allowed sample byte and unwritten flash)
  {
    while( 0xff!=next_sample ) {;}            //wait for next_sample buffer to be free (0xFF is a not allowed sample byte))
    next_sample = t;                          //set next_sample buffer
    t = pdkspi_sendreceive(0xff);             //fetch next sample to temp
  }

  __disgint();                                //global disable interrupt

  AMP_NSHDN_PORT &= ~(1<<AMP_NSHDN_PIN);      //enable shutdown of amp
  pdkdelay(10);                               //wait 10 msec for amp to turn off softly

  T16M = T16_CLK_DISABLE;                     //disable T16
  TM2C = 0;                                   //disable PWM

  pdkspi_sendreceive(0xB9);                   //send power down to flash (must be followed by deselect)
  pdkspi_deselect();                          //deselect flash chip, stop read

  for(;;)                                     //sleep for ever
  {
    __stopsys();
  }
}
