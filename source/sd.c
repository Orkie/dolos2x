#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "dolos2x.h"

static volatile uint16_t* rGPIOIPINLVL = NULL;
static volatile uint16_t* rSDICmdSta = NULL;
static volatile uint16_t* rSDICmdCon = NULL;
static volatile uint16_t* rSDIRSP0 = NULL;
static volatile uint16_t* rSDIRSP1 = NULL;
static volatile uint16_t* rSDIRSP2 = NULL;
static volatile uint16_t* rSDIRSP3 = NULL;
static volatile uint16_t* rSDIRSP4 = NULL;
static volatile uint16_t* rSDIRSP5 = NULL;
static volatile uint16_t* rSDIRSP6 = NULL;
static volatile uint16_t* rSDIRSP7 = NULL;

static volatile uint16_t nextSDICmdSta = 0x0;

typedef enum {
  NOT_READY,
  READY
} SdState;

static SdState state = NOT_READY;
static bool nextCmdIsAcmd = false;

static uint32_t r1 = BIT(30); // SDHC, page 49 of SD pdf

#define CARD_RCA 0x1234

void shortResponse(uint32_t resp) {
  *rSDIRSP0 = resp & 0xFFFF;
  *rSDIRSP1 = (resp >> 16) & 0xFFFF;
  nextSDICmdSta |= BIT(9);
}

void longResponse(uint16_t sp0, uint16_t sp1, uint16_t sp2, uint16_t sp3, uint16_t sp4, uint16_t sp5, uint16_t sp6, uint16_t sp7) {
  *rSDIRSP0 = sp0;
  *rSDIRSP1 = sp1;
  *rSDIRSP2 = sp2;
  *rSDIRSP3 = sp3;
  *rSDIRSP4 = sp4;
  *rSDIRSP5 = sp5;
  *rSDIRSP6 = sp6;
  *rSDIRSP7 = sp7;
  nextSDICmdSta |= BIT(9);
}

void r6Response() {
  shortResponse(CARD_RCA<<16);
}

void r1Response() {
  shortResponse(r1);
}

void r1bResponse() {
  shortResponse(r1);
}

void r2Response() {
  longResponse(0, 0, 0, 0, 0, 0, 0, 0);
}

static int cmd0() {
  state = NOT_READY;
  nextCmdIsAcmd = false;
  r1 &= ~BIT(31);
}

static int cmd8() {
  r1Response();
  state = READY;
  r1 |= BIT(31);
}

static int cmd55() {
  nextCmdIsAcmd = true;
  r1Response();
}

static int acmd41() {
  r1Response();
}

static int cmd2() {
  longResponse(0, 0, 0, 0, 0, 0, 0, 0);
}

static int cmd3() {
  r6Response();
}

static int cmd9() {
  r2Response();
}

static int cmd7() {
  r1bResponse();
}

static void handleSDICmdSta(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
  if(type == UC_MEM_WRITE) {
    nextSDICmdSta = CLEARBITS(*rSDICmdSta, (uint16_t)value);
  } else {
    *rSDICmdSta = nextSDICmdSta;
  }
}

static void handleSDICmdCon(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
  uint8_t cmd = value & 0x3F;
  printf("DOLOS: SD command %d\n", cmd);
  *rSDICmdCon = (uint16_t) value;
  if(value & BIT(8)) {
    nextSDICmdSta = BIT(11);
    
    switch(cmd) {
    case 0:
      cmd0();
      break;
    case 2:
      cmd2();
      break;
    case 3:
      cmd3();
      break;
    case 7:
      cmd7();
      break;
    case 8:
      cmd8();
      break;
    case 9:
      cmd9();
      break;
    case 55:
      cmd55();
      break;
    case 41:
      if(nextCmdIsAcmd) {
	acmd41();
      }
      break;
    }
  }
}

int initSD() {
  rGPIOIPINLVL = ((uint16_t*)getIORegs())+(GPIOIPINLVL>>1);
  rSDICmdSta = ((uint16_t*)getIORegs())+(SDICmdSta>>1);
  rSDICmdCon = ((uint16_t*)getIORegs())+(SDICmdCon>>1);
  rSDIRSP0 = ((uint16_t*)getIORegs())+(SDIRSP0>>1);
  rSDIRSP1 = ((uint16_t*)getIORegs())+(SDIRSP1>>1);
  rSDIRSP2 = ((uint16_t*)getIORegs())+(SDIRSP2>>1);
  rSDIRSP3 = ((uint16_t*)getIORegs())+(SDIRSP3>>1);
  rSDIRSP4 = ((uint16_t*)getIORegs())+(SDIRSP4>>1);
  rSDIRSP5 = ((uint16_t*)getIORegs())+(SDIRSP5>>1);
  rSDIRSP6 = ((uint16_t*)getIORegs())+(SDIRSP6>>1);
  rSDIRSP7 = ((uint16_t*)getIORegs())+(SDIRSP7>>1);

  *rGPIOIPINLVL = ~BIT(14);
  *rSDICmdSta = 0x0;

  hookRegWrite(REG(SDICmdCon), 2, handleSDICmdCon);
  hookRegRW(REG(SDICmdSta), 2, handleSDICmdSta);

  return 0;
}
