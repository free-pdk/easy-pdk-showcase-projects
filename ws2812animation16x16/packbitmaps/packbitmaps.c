//
// compile: gcc packbitmaps.c -o packbitmaps
//
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "bitmaps/digdug.h"
#include "bitmaps/frogger.h"
#include "bitmaps/galaga.h"
#include "bitmaps/pacman.h"
#include "bitmaps/smb.h"
#include "bitmaps/qbert.h"

#define PICSIZ   (16*16)
#define MAXPAL   8
#define MAXRLE   31
#define RLESHIFT 3
#define ZIGZAG   16

static uint32_t pal[MAXPAL];
static uint32_t pal_len = 0;

bool buildPalette(const uint32_t* picARGB, const uint32_t len)
{
  for( uint32_t p=0; p<len; p++ )
  {
    uint32_t pp;
    for( pp=0; pp<pal_len; pp++ )
      if( pal[pp] == picARGB[p] )
        break;
    if( pp==pal_len )
    {
      if( pal_len<MAXPAL )
      {
        pal[pal_len] = picARGB[p];
        pal_len++;
      }
      else return false;
    }
  }
  return true;
}

bool convertToPalette(const uint32_t* picARGB, const uint32_t len, uint8_t* outPaletteImage)
{
  bool zigzagrev = false;

  for( uint32_t p=0; p<len; p++ )
  {
    uint32_t pp;
    for( pp=0; pp<pal_len; pp++ )
      if( pal[pp] == picARGB[p] )
        break;
    if( pp==pal_len )
      return false;

    //pp contains pointer of pixel to palette
    if( zigzagrev )
      outPaletteImage[p] = pp;
    else
      outPaletteImage[((p/ZIGZAG)*ZIGZAG)+ZIGZAG-((p%ZIGZAG)+1)] = pp;

    if( 15 == (p%16) )
      zigzagrev=!zigzagrev;
  }
  return true;
}

uint32_t compressPicRLE(const uint8_t* paletteImage, const uint32_t len, uint8_t* outImageRLE)
{
  uint32_t cp=0;
  for( uint32_t zp=0; zp<len; )
  {
    //how many?
    uint32_t mp;
    for( mp=0; (mp+zp<len) && (mp<=MAXRLE); mp++ )
      if( paletteImage[zp+mp] != paletteImage[zp] )
       break;
    outImageRLE[cp++] = ((mp-1)<<RLESHIFT) | paletteImage[zp];
    zp+=mp;
  }
  return cp;
}

uint32_t squeeze(const uint32_t** pics, const uint32_t piccount, const char* szName)
{
  pal_len = 0;
  for( uint32_t pidx=0; pidx<piccount; pidx++ )
  {
    if( !buildPalette( pics[pidx], PICSIZ ) )
    {
      printf("ERROR: Palette exceeds maximum of %d\n", MAXPAL);
      return 0;
    }
  }

  printf("const uint8_t %s_pal[%d] = {", szName, pal_len*3);
  for( uint32_t pp=0; pp<pal_len; pp++ )
    printf("0x%02x,0x%02x,0x%02x,", (pal[pp]>>16)&0xFF, (pal[pp]>>8)&0xFF, pal[pp]&0xFF );
  printf("};\n");

  uint32_t lentot = pal_len*3;

  for( uint32_t pidx=0; pidx<piccount; pidx++ )
  {
    uint8_t paletteImage[PICSIZ];
    if( !convertToPalette( pics[pidx], PICSIZ, paletteImage ) )
    {
      printf("ERROR: Image not mapped to palette\n");
      return 0;
    }

    uint8_t rleImage[PICSIZ];
    uint32_t rleLen = compressPicRLE(paletteImage, PICSIZ, rleImage);

    printf("const uint8_t %s_rle_%d[%d] = {", szName, pidx, rleLen);
    for( uint32_t yp=0; yp<rleLen; yp++ )
      printf("0x%02x,", rleImage[yp] );
    printf("};\n");
    
    lentot += rleLen;
  }
  
  return lentot;
}

int main( int argc, const char * argv [] )
{
  uint32_t lentot = 0;
  const uint32_t* digdug[]  = { DigDugTaizo01, DigDugTaizo02, DigDugTaizoShovel01, DigDugTaizoShovel02, DigDugPooka01, DigDugPooka02, DigDugFygar01, DigDugFygar02 };
  lentot += squeeze( digdug, sizeof(digdug)/sizeof(uint32_t*), "digdug" );

  const uint32_t* frogger[] = { Frogger01, Frogger02 };
  lentot += squeeze( frogger, sizeof(frogger)/sizeof(uint32_t*), "frogger" );

  const uint32_t* galaga[]  = { GalagaShip, GalagaBoss01, GalagaBoss02, GalagaGoei01, GalagaGoei02, GalagaZako01, GalagaZako02 };
  lentot += squeeze( galaga, sizeof(galaga)/sizeof(uint32_t*), "galaga" );

  const uint32_t* pacman[]  = { PacManPac01, PacManPac02, PacManPac03, PacManMsPac01, PacManMsPac02, PacManMsPac03 };
  lentot += squeeze( pacman, sizeof(pacman)/sizeof(uint32_t*), "pacman" );

  const uint32_t* pacmanmb[]  = { PacManBlinky01, PacManBlinky02 };
  lentot += squeeze( pacmanmb, sizeof(pacmanmb)/sizeof(uint32_t*), "pacmanmb" );
  const uint32_t* pacmanmp[]  = { PacManPinky01, PacManPinky02 };
  lentot += squeeze( pacmanmp, sizeof(pacmanmp)/sizeof(uint32_t*), "pacmanmp" );
  const uint32_t* pacmanmi[]  = { PacManInky01, PacManInky02 };
  lentot += squeeze( pacmanmi, sizeof(pacmanmi)/sizeof(uint32_t*), "pacmanmi" );
  const uint32_t* pacmanmc[]  = { PacManClyde01, PacManClyde02 };
  lentot += squeeze( pacmanmc, sizeof(pacmanmc)/sizeof(uint32_t*), "pacmanmc" );
  const uint32_t* pacmanmg[]  = { PacManGhost01, PacManGhost02 };
  lentot += squeeze( pacmanmg, sizeof(pacmanmg)/sizeof(uint32_t*), "pacmanmg" );

  const uint32_t* qbert[] = { Qbert01, Qbert02 };
  lentot += squeeze( qbert, sizeof(qbert)/sizeof(uint32_t*), "qbert" );

  const uint32_t* smbm[]  = { SMBMario01, SMBMario02, SMBMario03, SMBMario04 };
  lentot += squeeze( smbm, sizeof(smbm)/sizeof(uint32_t*), "smbm" );

  const uint32_t* smbo[]  = { SMBGoomba01, SMBGoomba02 };
  lentot += squeeze( smbo, sizeof(smbo)/sizeof(uint32_t*), "smbo" );

  printf("lentot: %d\n",lentot);
  return 0;
}
