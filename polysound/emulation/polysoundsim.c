#include <stdint.h>

// declare that we implement SEND_SAMPLE function
#define SEND_SAMPLE SEND_SAMPLE
void SEND_SAMPLE(uint8_t sample);

// rename main function to pdk_main and setup macros to emulate SDCC compatibility
#define PFS173
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
#include "../polysound.c"                           // include the original source
#pragma pop_macro("main")
#pragma pop_macro("__asm__")

//// EMULATION

#include <stdio.h>
#include <SDL.h>

#define WAV_BUFFER_SIZE 4096
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

//emulation of IC version of SEND_SAMPLE function
void SEND_SAMPLE(uint8_t sample) {
  wav_buffer[wav_buffer_pos++] = sample;            //store the sample in wav buffer
  if( wav_buffer_pos >= wav_buffer_chunk ) {        //check if buffer was filled completely
    SDL_SemPost(wav_buffer_full);                   //signal wav buffer is filled
    SDL_SemWait(wav_buffer_empty);                  //wait until wav buffer was consumed
    wav_buffer_pos = 0;                             //reset buffer
  }
  TM3CT++;                                          //simulate next TM3 tick
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
  SDL_AudioSpec sdlas = { .freq=19531, .format=AUDIO_U8, .channels=1, .samples=WAV_BUFFER_SIZE, .callback=_emu_AudioCallback };
  SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &sdlas, NULL, 0);
  SDL_PauseAudioDevice(dev, 0);

  _sdcc_external_startup();                         //simulate call to _sdcc_external_startup()
  TM3CT++;                                          //simulate first TM3 tick
  pdk_main();                                       //simulate jump to main()

  return 0;
}
