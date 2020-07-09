#include "pdkdelay.h"
#include "easypdk/pdk.h"

void pdkdelay(uint32_t msec)
{
  msec<<=9; //*512, only works correct @4MHz SYSCLOCK
  for( ; msec>0; msec-- );
}
