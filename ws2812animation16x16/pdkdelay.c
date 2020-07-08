#include "pdkdelay.h"
#include "easypdk/pdk.h"

void pdkdelay(uint32_t msec)
{
  msec<<=10; //*1024, only works correct @8MHz SYSCLOCK
  for( ; msec>0; msec-- );
}
