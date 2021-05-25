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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>

#include "microemu_pfs154.h"

#define EMULATION_FILE "../build/noiseplug.PFS154.ihx"
#define TM2INT_SYSCLK_CYCLES 256                    //timer interrupt simulation (every 256 SYS_CLK = every 512 IHRC_CLK)

#define WAV_BUFFER_SIZE      4096
static uint8_t  wav_buffer[WAV_BUFFER_SIZE];        //buffer to hold the samples
static uint16_t wav_buffer_pos = 0;                 //buffer write index
static uint16_t wav_buffer_chunk = WAV_BUFFER_SIZE; //actual length of buffer used by system (might be WAV_BUFFER_SIZE/2)
static SDL_sem* wav_buffer_empty;                   //semaphore to signal empty wav buffer
static SDL_sem* wav_buffer_full;                    //semaphore to signal full wav buffer

//audio callback for SDL to fill in next audio buffer
static void _emu_AudioCallback(void*  userdata, uint8_t* stream, int len) {
  wav_buffer_chunk = len;                           //update chunk length
  SDL_SemWait(wav_buffer_full);                     //wait for full wav buffer
  SDL_memcpy(stream, wav_buffer, len);              //copy wav buffer to output stream
  SDL_SemPost(wav_buffer_empty);                    //signal wav buffer is empty
}

//set next sample in audio buffer, halt execution until free space in audio buffer exists
void _emu_SetSample(uint8_t sample) {
  wav_buffer[wav_buffer_pos++] = sample;            //store the sample in wav buffer
  if( wav_buffer_pos >= wav_buffer_chunk ) {        //check if buffer was filled completely
    SDL_SemPost(wav_buffer_full);                   //signal wav buffer is filled
    SDL_SemWait(wav_buffer_empty);                  //wait until wav buffer was consumed
    wav_buffer_pos = 0;                             //reset buffer
  }
}

//glue code to start SDL2 audio and PDK emulation
int main(int argc, char* argv[]) {
  if( SDL_Init(SDL_INIT_AUDIO)<0 ) {                //init SDL audio
    fprintf(stderr, "ERROR: SDL: %s\n", SDL_GetError());
    return -1;
  }
  atexit(SDL_Quit);

  //setup audio device and start playback
  wav_buffer_empty = SDL_CreateSemaphore(0);        //create semaphore
  wav_buffer_full  = SDL_CreateSemaphore(0);        //create semaphore
  SDL_AudioSpec sdlas = { .freq=32000, .format=AUDIO_U8, .channels=1, .samples=WAV_BUFFER_SIZE, .callback=_emu_AudioCallback };
  SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &sdlas, NULL, 0);
  SDL_PauseAudioDevice(dev, 0);

  //load the HEX file and emulate the cpu
  struct emuCPU cpu;
  if( emuCPUloadIHEX(&cpu, EMULATION_FILE) < 0 ) {
    fprintf(stderr, "ERROR: could not load: %s\n", EMULATION_FILE);
    return -1;
  }

  for(;;) {
    //execute instruction until TM2INT_SYSCLK_CYCLES cycles are used
    switch( emuCPUrun(&cpu,TM2INT_SYSCLK_CYCLES) ) {
      case 1: emuCPUreset(&cpu,false); break;       //RESET
      case 2: return 0;                             //STOPSYS
      case 3: break;                                //STOPEXE
      default: break;
    }

    static uint64_t nextTimerCycle = 0;
    if( cpu.eeCurrentCycle >= nextTimerCycle ) {
      nextTimerCycle += TM2INT_SYSCLK_CYCLES;
      cpu.eIO[0x05] = 0x40;                         //simulate INTRQ = INTRQ_TM2
      _emu_SetSample( cpu.eIO[0x09] );              //send sample found in TM2B (PFS154)
    }
  }

  return 0;
}
