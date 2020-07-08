#include "pdkws2812.h"
#include "pdkws2812_pins.h"
#include "easypdk/pdk.h"

void pdkws2812_init(void)
{
  WS2812_PORTC |= 1<<WS2812_PIN;
}

//adopted from cpldcpu changes: disable interrupts while sending, replaced swapc since it is not available on 13 bit variants
void pdkws2812_send_8bit(uint8_t val)
{
  (void)val; //val is referenced from asm later, compiler can not see this and complains, so we declare it used manually
  __asm
      disgint
      mov a, #8
1$:
      set1 _ASMS(WS2812_PORT), #(WS2812_PIN)             ;0
      sl _pdkws2812_send_8bit_PARM_1                     ;1
      t1sn f, c                                          ;2
      set0 _ASMS(WS2812_PORT), #(WS2812_PIN)             ;3
      nop                                                ;4
      nop                                                ;5
      nop                                                ;6
      set0 _ASMS(WS2812_PORT), #(WS2812_PIN)             ;7
      dzsn a                                             ;8
      goto 1$                                            ;10
      engint
  __endasm;
}
