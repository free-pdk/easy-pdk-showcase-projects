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

#include "microemu_pfs154.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define eA                          cpu->eeA
#define ePC                         cpu->eePC
#define eF                          (*((volatile uint8_t*)&cpu->eIO[0x00]))
#define eSP                         (*((volatile uint8_t*)&cpu->eIO[0x02]))
#define eGINTEnabled                cpu->eeGINTEnabled
#define eInterruptActive            cpu->eeInterruptActive
#define eT16                        cpu->eeT16
#define eCycle                      cpu->eeCurrentCycle
#define eCycles                     cpu->eeLastCycles

#define OCM(oc)                     (oc>>7)
#define a6(oc)                      (oc&0x3F)
#define a7(oc)                      (oc&0x7F)
#define a7w(oc)                     (oc&0x7E)
#define k(oc)                       ((uint8_t)oc)
#define bit(oc)                     (1<<((oc>>6)&7))
#define m(cpu,oc)                   (memg(cpu,a7(oc)))

#define iog(cpu,addr)               (cpu->eIO[addr&(IO_MAX-1)])
#define iop(cpu,addr,dat)           {cpu->eIO[addr&(IO_MAX-1)]=dat;}
#define memg(cpu,addr)              (cpu->eMem[addr&(MEM_MAX-1)])
#define memp(cpu,addr,dat)          {cpu->eMem[addr&(MEM_MAX-1)]=dat;}
#define codeg(cpu,addr)             (cpu->eCode[addr&(CODEW_MAX-1)])
#define push(cpu,dat)               {memp(cpu,eSP++,dat );}
#define pop(cpu)                    (memg(cpu,--eSP))
#define pushw(cpu,dat)              {push(cpu,dat);push(cpu,dat>>8);}
#define popw(cpu)                   ((((uint16_t)pop(cpu))<<8)|pop(cpu))

//#define FULL_FLAGS
#define fzrp(a,b,c)                 (!(((a&0xFF)+(b&0xFF)+c)&0xFF))
#define fzrm(a,b,c)                 (!(((a&0xFF)-(b&0xFF)-c)&0xFF))
#define fcyp(a,b,c)                 ((((a&0xFF)+(b&0xFF)+c)>>8)&1)
#define fcym(a,b,c)                 ((((a&0xFF)-(b&0xFF)-c)>>8)&1)
#ifdef FULL_FLAGS
#define facp(a,b,c)                 ((((a&0xF)+(b&0xF)+c)>>4)&1)
#define facm(a,b,c)                 (((a&0xF)<((b&0xF)+c))?1:0)
#define fovp(a,b,c)                 (((((a&0x7F)+(b&0x7F)+c)>>7)&1)^fcyp(a,b,c))
#define fovm(a,b,c)                 (((((a&0x7F)-(b&0x7F))>>7)&1)^fcym(a,b,c))
#else
#define facp(a,b,c)                 0
#define fovp(a,b,c)                 0
#define facm(a,b,c)                 0
#define fovm(a,b,c)                 0
#endif
#define flagsadd(a,b,c)             (0xF0|(fovp(a,b,c)<<3)|(facp(a,b,c)<<2)|(fcyp(a,b,c)<<1)|fzrp(a,b,c))
#define flagssub(a,b,c)             (0xF0|(fovm(a,b,c)<<3)|(facm(a,b,c)<<2)|(fcym(a,b,c)<<1)|fzrm(a,b,c))

int emuCPUrun(struct emuCPU *cpu, const uint64_t cycles) {

  for( uint64_t lastCycle=eCycle+cycles; eCycle<lastCycle; ) {

    if( eGINTEnabled && (cpu->eIO[0x04]&cpu->eIO[0x05]) ) {                                                                               //check if we need to enter ISR
      eInterruptActive = true;
      eGINTEnabled=false;
      pushw(cpu, ePC);
      ePC = 0x0010;
      eCycle+=2;
      return 0;
    }

    uint16_t oc = codeg(cpu,ePC++)&0x3FFF;                                                                                                //fetch next opcode and advance PC
    eCycle++;                                                                                                                             //increment current cycle counter

    int16_t T,M;                                                                                                                          //temp register

    switch( OCM(oc) ) {
      case OCM(0x0000):
        switch(oc&0x00FF) {
          case 0x0000: break;                                                                                                             //NOP
          case 0x0006: eA=codeg(cpu,memg(cpu,eSP)|(((uint16_t)memg(cpu,eSP+1))<<8))&0xFF;eCycle++;break;                                  //LDSPTL
          case 0x0007: eA=codeg(cpu,memg(cpu,eSP)|(((uint16_t)memg(cpu,eSP+1))<<8))>>8;eCycle++;break;                                    //LDSPTH
          case 0x0060: T=(eF>>1)&1;eF=flagsadd(eA,0,T);eA=(eA+T)&0xFF;break;                                                              //ADDC A
          case 0x0061: T=(eF>>1)&1;eF=flagssub(eA,0,T);eA=(eA-T)&0xFF;break;                                                              //SUBC A
          case 0x0062: eF=flagsadd(eA,1,0);eA=(eA+1)&0xFF;if(!eA){ePC++;eCycle++;}break;                                                  //IZSN A
          case 0x0063: eF=flagssub(eA,1,0);eA=(eA-1)&0xFF;if(!eA){ePC++;eCycle++;}break;                                                  //DZSN A
          case 0x0067: ePC=(ePC-1)+eA;break;                                                                                              //PCADD A
          case 0x0068: eA=(~eA)&0xFF;eF=(eF&0xFE)|(!eA);break;                                                                            //NOT A
          case 0x0069: eA=(-((int8_t)eA))&0xFF;eF=(eF&0xFE)|(!eA);break;                                                                  //NEG A
          case 0x006A: eF=(eF&0xFD)|((eA<<1)&2);eA>>=1;break;                                                                             //SR A
          case 0x006B: eF=(eF&0xFD)|((eA>>6)&2);eA=(eA<<1)&0xFF;break;                                                                    //SL A
          case 0x006C: eA|=(eF&2)<<7;eF=(eF&0xFD)|((eA<<1)&2);eA>>=1;break;                                                               //SRC A
          case 0x006D: T=(eF>>1)&1;eF=(eF&0xFD)|((eA>>6)&2);eA=((eA<<1)&0xFF)|T;break;                                                    //SLC A
          case 0x006E: eA=((eA<<4)|(eA>>4))&0xFF;break;                                                                                   //SWAP A
          case 0x0070: break;                                                                                                             //WDRESET
          case 0x0072: push(cpu,eA);push(cpu,eF);break;                                                                                   //PUSHAF
          case 0x0073: eF=pop(cpu);eA=pop(cpu);break;                                                                                     //POPAF
          case 0x0075: return 1;                                                                                                          //RESET
          case 0x0076: return 2;                                                                                                          //STOPSYS
          case 0x0077: return 3;                                                                                                          //STOPEXE
          case 0x0078: eGINTEnabled=true;break;                                                                                           //ENGINT
          case 0x0079: eGINTEnabled=false;break;                                                                                          //DISGINT
          case 0x007A: ePC=popw(cpu);break;                                                                                               //RET
          case 0x007B: ePC=popw(cpu);eGINTEnabled=true;eInterruptActive=false;break;                                                      //RETI
          case 0x007C: /*TODO*/ break;                                                                                                    //MUL
        }
        break;

      case OCM(0x0080):
        if(oc&0x00C0) {iop(cpu,a6(oc),iog(cpu,a6(oc))^eA);}                                                                               //XOR IO, A
        break;

      case OCM(0x0100): T=memg(cpu,a7w(oc));eCycle++;if(oc&1)eA=codeg(cpu,T);else eA=codeg(cpu,T)>>8;break;                               //LDTABL / LDTABH

      case OCM(0x0180):
        switch(oc&0xFFC0) {
          case 0x0180: iop(cpu,a6(oc),eA);break;                                                                                          //MOV IO, A
          case 0x01C0: eA=iog(cpu,a6(oc));eF=(eF&0xFE)|(!eA);break;                                                                       //MOV A, IO
        }
        break;

      case OCM(0x0200) ... OCM(0x02FF): eA=k(oc);ePC=popw(cpu);break;                                                                     //RET k

      case OCM(0x0300):
        switch(oc&1) {
          case 0: memp(cpu,a7w(oc),eT16&0xFF);memp(cpu,a7w(oc),eT16>>8);break;                                                            //STT16 M
          case 1: eT16=memg(cpu,a7w(oc))|(((uint16_t)memg(cpu,a7w(oc)+1))<<8);break;                                                      //LDT16 M
        }
        break;

      case OCM(0x0380): T=memg(cpu,a7w(oc));eCycle++;if(oc&1)eA=memg(cpu,T);else memp(cpu,T,eA);break;                                    //IDXM A,M / IDXM M,A
      case OCM(0x0400) ... OCM(0x05FF): T=iog(cpu,a6(oc));iop(cpu,a6(oc),eF&2?T|bit(oc):T&~bit(oc));eF=(eF&0xFD)|((T&bit(oc))?2:0);break; //SWAPC IO.n
      case OCM(0x0600): eF=flagssub(eA,m(cpu,oc),0);break;                                                                                //COMP A, M
      case OCM(0x0680): eF=flagssub(m(cpu,oc),eA,0);break;                                                                                //COMP M, A
      case OCM(0x0700): M=m(cpu,oc);eF=flagsadd(eA,(-M)&0xFF,0);eA=(eA+((-M)&0xFF))&0xFF;break;                                           //NADD A, M
      case OCM(0x0780): M=m(cpu,oc);eF=flagsadd(M,(-eA)&0xFF,0);memp(cpu,a7(oc),(M+((-eA)&0xFF))&0xFF);break;                             //NADD M, A
      case OCM(0x0800): M=m(cpu,oc);eF=flagsadd(M,eA,0);memp(cpu,a7(oc),M+eA);break;                                                      //ADD M, A
      case OCM(0x0880): M=m(cpu,oc);eF=flagssub(M,eA,0);memp(cpu,a7(oc),M-eA);break;                                                      //SUB M, A
      case OCM(0x0900): M=m(cpu,oc);T=(eF>>1)&1;eF=flagsadd(M,eA,T);memp(cpu,a7(oc),M+eA+T);break;                                        //ADDC M, A
      case OCM(0x0980): M=m(cpu,oc);T=(eF>>1)&1;eF=flagssub(M,eA,T);memp(cpu,a7(oc),M-eA-T);break;                                        //SUBC M, A
      case OCM(0x0A00): M=m(cpu,oc)&eA;memp(cpu,a7(oc),M);eF=(eF&0xFE)|(!M);break;                                                        //AND M, A
      case OCM(0x0A80): M=m(cpu,oc)|eA;memp(cpu,a7(oc),M);eF=(eF&0xFE)|(!M);break;                                                        //OR M, A
      case OCM(0x0B00): M=m(cpu,oc)^eA;memp(cpu,a7(oc),M);eF=(eF&0xFE)|(!M);break;                                                        //XOR M, A
      case OCM(0x0B80): memp(cpu,a7(oc),eA);break;                                                                                        //MOV M, A
      case OCM(0x0C00): M=m(cpu,oc);eF=flagsadd(eA,M,0);eA=(eA+M)&0xFF;break;                                                             //ADD A, M
      case OCM(0x0C80): M=m(cpu,oc);eF=flagssub(eA,M,0);eA=(eA-M)&0xFF;break;                                                             //SUB A, M
      case OCM(0x0D00): M=m(cpu,oc);T=(eF>>1)&1;eF=flagsadd(eA,M,T);eA=(eA+M+T)&0xFF;break;                                               //ADDC A, M
      case OCM(0x0D80): M=m(cpu,oc);T=(eF>>1)&1;eF=flagssub(eA,M,T);eA=(eA-M-T)&0xFF;break;                                               //SUBC A, M
      case OCM(0x0E00): eA=eA&m(cpu,oc);eF=(eF&0xFE)|(!eA);break;                                                                         //AND A, M
      case OCM(0x0E80): eA=eA|m(cpu,oc);eF=(eF&0xFE)|(!eA);break;                                                                         //OR A, M
      case OCM(0x0F00): eA=eA^m(cpu,oc);eF=(eF&0xFE)|(!eA);break;                                                                         //XOR A, M
      case OCM(0x0F80): eA=m(cpu,oc);eF=(eF&0xFE)|(!eA);break;                                                                            //MOV A, M
      case OCM(0x1000): M=m(cpu,oc);T=(eF>>1)&1;eF=flagsadd(M,0,T);memp(cpu,a7(oc),M+T);break;                                            //ADDC M
      case OCM(0x1080): M=m(cpu,oc);T=(eF>>1)&1;eF=flagssub(M,0,T);memp(cpu,a7(oc),M-T);break;                                            //SUBC M
      case OCM(0x1100): M=m(cpu,oc);eF=flagsadd(M,1,0);memp(cpu,a7(oc),M+1);if(eF&1){ePC++;eCycle++;}break;                               //IZSN M
      case OCM(0x1180): M=m(cpu,oc);eF=flagssub(M,1,0);memp(cpu,a7(oc),M-1);if(eF&1){ePC++;eCycle++;}break;                               //DZSN M
      case OCM(0x1200): M=m(cpu,oc);eF=flagsadd(M,1,0);memp(cpu,a7(oc),M+1);break;                                                        //INC M
      case OCM(0x1280): M=m(cpu,oc);eF=flagssub(M,1,0);memp(cpu,a7(oc),M-1);break;                                                        //DEC M
      case OCM(0x1300): memp(cpu,a7(oc),0);break;                                                                                         //CLEAR M
      case OCM(0x1380): M=m(cpu,oc);memp(cpu,a7(oc),eA);eA=M;break;                                                                       //XCH A,M
      case OCM(0x1400): M=(~m(cpu,oc))&0xFF;eF=(eF&0xFE)|(!M);memp(cpu,a7(oc),M);break;                                                   //NOT M
      case OCM(0x1480): M=(-((int8_t)m(cpu,oc)))&0xFF;eF=(eF&0xFE)|(!M);memp(cpu,a7(oc),M);break;                                         //NEG M
      case OCM(0x1500): M=m(cpu,oc);eF=(eF&0xFD)|((M<<1)&2);memp(cpu,a7(oc),M>>1);break;                                                  //SR M
      case OCM(0x1580): M=m(cpu,oc);eF=(eF&0xFD)|((M>>6)&2);memp(cpu,a7(oc),M<<1);break;                                                  //SL M
      case OCM(0x1600): M=m(cpu,oc)|(eF&2)<<7;eF=(eF&0xFD)|((M<<1)&2);memp(cpu,a7(oc),M>>1);break;                                        //SRC M
      case OCM(0x1680): M=m(cpu,oc);T=(eF>>1)&1;eF=(eF&0xFD)|((M>>6)&2);memp(cpu,a7(oc),(M<<1)|T);break;                                  //SLC M
      case OCM(0x1700): M=m(cpu,oc);eF=flagssub(eA,M,0);if(!((eA-M)&0xFF)){ePC++;eCycle++;}break;                                         //CEQSN A,M
      case OCM(0x1780): M=m(cpu,oc);eF=flagssub(eA,M,0);if((eA-M)&0xFF){ePC++;eCycle++;}break;                                            //CNEQSN A,M
      case OCM(0x1800) ... OCM(0x19FF): if(!(iog(cpu,a6(oc))&bit(oc))){ePC++;eCycle++;}break;                                             //T0SN IO.n
      case OCM(0x1A00) ... OCM(0x1BFF): if(iog(cpu,a6(oc))&bit(oc)){ePC++;eCycle++;}break;                                                //T1SN IO.n
      case OCM(0x1C00) ... OCM(0x1DFF): iop(cpu,a6(oc),iog(cpu,a6(oc))&~bit(oc));break;                                                   //SET0 IO.n
      case OCM(0x1E00) ... OCM(0x1FFF): iop(cpu,a6(oc),iog(cpu,a6(oc))|bit(oc));break;                                                    //SET1 IO.n
      case OCM(0x2000) ... OCM(0x21FF): if(!(memg(cpu,a6(oc))&bit(oc))){ePC++;eCycle++;}break;                                            //T0SN M.n
      case OCM(0x2200) ... OCM(0x23FF): if(memg(cpu,a6(oc))&bit(oc)){ePC++;eCycle++;}break;                                               //T1SN M.n
      case OCM(0x2400) ... OCM(0x25FF): memp(cpu,a6(oc),memg(cpu,a6(oc))&~bit(oc));break;                                                 //SET0 M.n
      case OCM(0x2600) ... OCM(0x27FF): memp(cpu,a6(oc),memg(cpu,a6(oc))|bit(oc));break;                                                  //SET1 M.n
      case OCM(0x2800) ... OCM(0x28FF): eF=flagsadd(eA,k(oc),0);eA=(eA+k(oc))&0xFF;break;                                                 //ADD A,k
      case OCM(0x2900) ... OCM(0x29FF): eF=flagssub(eA,k(oc),0);eA=(eA-k(oc))&0xFF;break;                                                 //SUB A,k
      case OCM(0x2A00) ... OCM(0x2AFF): eF=flagssub(eA,k(oc),0);if(!((eA-k(oc))&0xFF)){ePC++;eCycle++;}break;                             //CEQSN A,k
      case OCM(0x2B00) ... OCM(0x2BFF): eF=flagssub(eA,k(oc),0);if((eA-k(oc))&0xFF){ePC++;eCycle++;}break;                                //CNEQSN A,k
      case OCM(0x2C00) ... OCM(0x2CFF): eA&=k(oc);eF=(eF&0xFE)|(!eA);break;                                                               //AND A,k
      case OCM(0x2D00) ... OCM(0x2DFF): eA|=k(oc);eF=(eF&0xFE)|(!eA);break;                                                               //OR A,k
      case OCM(0x2E00) ... OCM(0x2EFF): eA^=k(oc);eF=(eF&0xFE)|(!eA);break;                                                               //XOR A,k
      case OCM(0x2F00) ... OCM(0x2FFF): eA=k(oc);break;                                                                                   //MOV A,k
      case OCM(0x3000) ... OCM(0x37FF): ePC=oc&0x07FF;eCycle++;break;                                                                     //GOTO p
      case OCM(0x3800) ... OCM(0x3FFF): pushw(cpu,ePC);ePC=oc&0x07FF;eCycle++;break;                                                      //CALL p
    }
  }
  return 0;
}

void emuCPUreset(struct emuCPU *cpu, bool clearRAM) {
  memset( cpu->eIO, 0xFF, IO_MAX );
  if( clearRAM )
    memset( cpu->eMem, 0x55, MEM_MAX );

  ePC = 0;
  eA = 0;
  eF = 0xF0;
  eGINTEnabled = false;
  eInterruptActive = false;
  eCycle = 0;
  eT16 = 0;

  //setup default clock
  cpu->eIO[0x03] = 0xF4;
}

//IHEX LOADER
static bool _X2UI8(const char* str, uint8_t* out) {
  char tmp[] = {str[0],str[1],0};
  unsigned int r;
  if( 1 != sscanf(tmp, "%x", &r) )
    return false;
  *out = r;
  return true;
}

static bool _FPDKIHEX8_ParseLine(const char* line, uint8_t* type, uint16_t* address, uint8_t* data, uint8_t* datcount) {
  uint8_t addrl, addrh;
  if( (strlen(line)<9) || (':' != line[0]) ||
      !_X2UI8(&line[1], datcount) || !_X2UI8(&line[3], &addrh) || !_X2UI8(&line[5], &addrl) || !_X2UI8(&line[7], type) )
    return false;
  *address = (((uint16_t)addrh)<<8) | addrl;

  uint8_t check;
  if( (strlen(line)<(9+2*(*datcount))) || !_X2UI8(&line[9+2*(*datcount)], &check) )
    return false;
  check += (*datcount) + addrl + addrh + (*type);

  for( uint8_t p=0; p<(*datcount); p++ )
  {
    if( !_X2UI8(&line[9+2*p], &data[p]) )
      return false;
    check += data[p];
  }
  return( 0 == check );
}

int FPDKIHEX8_ReadFile(const char* filename, uint16_t* datout, const uint16_t datcount) {
  memset(datout, 0, sizeof(uint16_t)*datcount);

  FILE *fin = fopen(filename, "r");
  if( !fin )
    return -1;

  bool berr = false;

  char line[600];
  while( NULL != fgets(line, sizeof(line), fin) ) {
    uint8_t tmp[256];
    uint8_t type;
    uint16_t address;
    uint8_t count;
    if( !_FPDKIHEX8_ParseLine(line, &type, &address, tmp, &count) || (type>1) ) {
      berr = true;
      break;
    }

    if( 1 == type )
      break;

    if( (address+count) > datcount ) {
      berr = true;
      break;
    }

    for( uint8_t p=0; p<count; p++ )
      datout[address+p] = 0x100 + tmp[p];
  }
  fclose(fin);

  if( berr )
    return -2;

  return 0;
}

int emuCPUloadIHEX(struct emuCPU *cpu, const char *filename) {
  uint16_t bin[0x2000];
  memset( bin, 0xff, 0x2000);
  if( FPDKIHEX8_ReadFile(filename, bin, 0x2000) < 0 )
    return -1; //error reading input file

  memset( cpu, 0, sizeof(struct emuCPU) );
  memset( cpu->eCode, 0xFF, CODEW_MAX*sizeof(uint16_t) );

  uint32_t len = 0;
  for( uint32_t p=0; p<CODEW_MAX; p++) {
    if( (bin[p*2] & 0xFF00) ||  (bin[p*2+1] & 0xFF00) ) {
      cpu->eCode[p] = (bin[p*2]&0xFF) | ((bin[p*2+1]&0xFF)<<8);
      len = p + 1;
    }
  }
  emuCPUreset( cpu, true );
  return 0;
}
