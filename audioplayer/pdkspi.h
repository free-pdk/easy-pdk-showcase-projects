#ifndef __PDKSPI_H__
#define __PDKSPI_H__

#include <stdint.h>
#include "pdkspi_pins.h"

void    pdkspi_init(void);

#define pdkspi_select()   { SPI_PORT &= ~(1<<SPI_NCS_PIN); } //NCS low (select)
#define pdkspi_deselect() { SPI_PORT |= (1<<SPI_NCS_PIN); }  //NCS high (deselect)

uint8_t pdkspi_sendreceive(uint8_t s);

#endif //__PDKSPI_H__

