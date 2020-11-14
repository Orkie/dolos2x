#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "dolos2x.h"

static FILE* sdFp;

static volatile uint16_t rGPIOIPINLVL = 0x0;
static volatile uint16_t rSDICmdSta = 0x0;
static volatile uint16_t rSDICmdCon = 0x0;
static volatile uint16_t rSDICmdArgL = 0x0;
static volatile uint16_t rSDICmdArgH = 0x0;
static volatile uint16_t rSDIFSTA = 0x0;
static volatile uint8_t rSDIDAT = 0x0;
static volatile uint16_t rSDIRSP0 = 0x0;
static volatile uint16_t rSDIRSP1 = 0x0;
static volatile uint16_t rSDIRSP2 = 0x0;
static volatile uint16_t rSDIRSP3 = 0x0;
static volatile uint16_t rSDIRSP4 = 0x0;
static volatile uint16_t rSDIRSP5 = 0x0;
static volatile uint16_t rSDIRSP6 = 0x0;
static volatile uint16_t rSDIRSP7 = 0x0;

static volatile uint16_t nextSDICmdSta = 0x0;

typedef enum {
  NOT_READY,
  READY,
  BLOCK_READ,
  BLOCK_WRITE
} SdState;

#define CARD_RCA 0x1234
#define BLOCK_SZ 512

static SdState state = NOT_READY;
static bool nextCmdIsAcmd = false;
static uint32_t blockCounter = 0;
static uint8_t blockBuffer[BLOCK_SZ];
static uint32_t blockNumber = 0;

static uint32_t r1 = BIT(30); // SDHC, page 49 of SD pdf

void shortResponse(uint32_t resp) {
  rSDIRSP0 = resp & 0xFFFF;
  rSDIRSP1 = (resp >> 16) & 0xFFFF;
  nextSDICmdSta |= BIT(9);
}

void longResponse(uint16_t sp0, uint16_t sp1, uint16_t sp2, uint16_t sp3, uint16_t sp4, uint16_t sp5, uint16_t sp6, uint16_t sp7) {
  rSDIRSP0 = sp0;
  rSDIRSP1 = sp1;
  rSDIRSP2 = sp2;
  rSDIRSP3 = sp3;
  rSDIRSP4 = sp4;
  rSDIRSP5 = sp5;
  rSDIRSP6 = sp6;
  rSDIRSP7 = sp7;
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
  longResponse(0, 1 << 14, 0, 0, 0, 0, 0, 0);
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

static int cmd18(uint32_t arg) {
  state = BLOCK_READ;
  blockCounter = 0;
  blockNumber = arg;
  r1Response();
}

static int cmd25(uint32_t arg) {
  state = BLOCK_WRITE;
  blockCounter = 0;
  blockNumber = arg;
  r1Response();
}

static int cmd12() {
  state = READY;
  r1bResponse();
}

static void handleGPIOIPINLVL(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = rGPIOIPINLVL;
  }
}

static void handleSDICmdSta(bool isRead, uint64_t* value) {
  if(isRead) {
      *value = nextSDICmdSta;
  } else {
    nextSDICmdSta = CLEARBITS(rSDICmdSta, (uint16_t)(*value));
  }
}

static void handleSDICmdCon(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = rSDICmdCon;
  } {
    uint8_t cmd = (*value) & 0x3F;
    uint32_t arg = (rSDICmdArgH << 16) | rSDICmdArgL;
#ifdef DEBUG
    printf("DOLOS: SD command %d, arg %d\n", cmd, arg);
#endif
    rSDICmdCon = (uint16_t) (*value);
    if((*value) & BIT(8)) {
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
      case 12:
	cmd12();
	break;
      case 18:
	cmd18(arg);
	break;
      case 25:
	cmd25(arg);
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
}

static void handleSDICmdArgL(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = rSDICmdArgL;
  } else {
    rSDICmdArgL = *value;
  }
}

static void handleSDICmdArgH(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = rSDICmdArgH;
  } else {
    rSDICmdArgL = *value;
  }
}

static void handleSDIFSTA(bool isRead, uint64_t* value) {
  if(isRead) {
    if(state == BLOCK_READ) {
      *value = 0x1;
    } else if(state == BLOCK_WRITE) {
      *value = BIT(13);
    }
  }
}

static void handleSDIDAT(bool isRead, uint64_t* value) {
  if(isRead) {
    if(blockCounter % BLOCK_SZ == 0) {
      memset(blockBuffer, 0x0, BLOCK_SZ);
      fseek(sdFp, blockNumber*BLOCK_SZ, SEEK_SET);
      fread(blockBuffer, 1, BLOCK_SZ, sdFp);
      blockNumber++;
    }
    *value = blockBuffer[blockCounter%BLOCK_SZ];
    blockCounter++;
  }
}

static void handleSDIRSP0(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = rSDIRSP0;
  }
}

static void handleSDIRSP1(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = rSDIRSP1;
  }
}

static void handleSDIRSP2(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = rSDIRSP2;
  }
}

static void handleSDIRSP3(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = rSDIRSP3;
  }
}

static void handleSDIRSP4(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = rSDIRSP4;
  }
}

static void handleSDIRSP5(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = rSDIRSP5;
  }
}

static void handleSDIRSP6(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = rSDIRSP6;
  }
}

static void handleSDIRSP7(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = rSDIRSP7;
  }
}

int initSD() {
  registerIoCallback(GPIOIPINLVL, handleGPIOIPINLVL);
  registerIoCallback(SDICmdSta, handleSDICmdSta);
  registerIoCallback(SDICmdCon, handleSDICmdCon);
  registerIoCallback(SDICmdArgL, handleSDICmdArgL);
  registerIoCallback(SDICmdArgH, handleSDICmdArgH);
  registerIoCallback(SDIFSTA, handleSDIFSTA);
  registerIoCallback(SDIDAT, handleSDIDAT);
  registerIoCallback(SDIRSP0, handleSDIRSP0);
  registerIoCallback(SDIRSP1, handleSDIRSP1);
  registerIoCallback(SDIRSP2, handleSDIRSP2);
  registerIoCallback(SDIRSP3, handleSDIRSP3);
  registerIoCallback(SDIRSP4, handleSDIRSP4);
  registerIoCallback(SDIRSP5, handleSDIRSP5);
  registerIoCallback(SDIRSP6, handleSDIRSP6);
  registerIoCallback(SDIRSP7, handleSDIRSP7);

  rGPIOIPINLVL = ~BIT(14);

  sdFp = fopen("sd.img", "rb+");
  if(sdFp == NULL) {
    fprintf(stderr, "Error opening SD image\n");
    return 1;
  }
  
  return 0;
}
