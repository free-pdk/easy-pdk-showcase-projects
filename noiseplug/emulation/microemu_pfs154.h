/*
Copyright (C) 2021  freepdk  https://free-pdk.github.io

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __INC_MICROEMU_PFS154_H__
#define __INC_MICROEMU_PFS154_H__

#include <stdint.h>
#include <stdbool.h>

#define IO_MAX    64
#define MEM_MAX   128
#define CODEW_MAX 2048

struct emuCPU {
  uint8_t  eIO[IO_MAX];                   //io (special register)
  uint8_t  eMem[MEM_MAX];                 //memory (RAM)
  uint16_t eCode[CODEW_MAX];              //instruction words (ROM)
  uint32_t eePC;                          //program counter
  int16_t  eeA;                           //A register
  uint16_t eeT16;                         //timer T16 register
  bool     eeGINTEnabled;                 //gloabl interrupt enabled
  bool     eeInterruptActive;             //internal status that cpu started interrupt
  uint64_t eeCurrentCycle;                //keeps track of current CPU cycle (can be used to synchronize to real time)
};

//load a IHEX file
int emuCPUloadIHEX(struct emuCPU *cpu, const char *filename);

//reset CPU
void emuCPUreset(struct emuCPU *cpu, bool clearRAM);

//run CPU - execute specififed cycles
int emuCPUrun(struct emuCPU *cpu, const uint64_t cycles);

#endif // __INC_MICROEMU_PFS154_H__
