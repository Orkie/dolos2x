#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "dolos2x.h"
#include "qemu/block.h"
#include "qemu/sd.h"

static BlockDevice* sdBlk;
static SDState *sd;
static uint8_t rspBuffer[16];

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

typedef enum {
  NOT_READY,
  READY,
  BLOCK_READ,
  BLOCK_WRITE
} SdState;

#define CARD_RCA 0x1234
#define BLOCK_SZ 512

static SdState state = NOT_READY;

void shortResponse(uint8_t* data) {
  rSDIRSP1 = data[0] | (data[1]<<8);
  rSDIRSP0 = data[2] | (data[3]<<8);

  rSDICmdSta |= BIT(9);
}

void longResponse(uint8_t* data) {
  rSDIRSP0 = data[3] | (data[2]<<8);
  rSDIRSP1 = data[1] | (data[0]<<8);
  rSDIRSP2 = data[5] | (data[4]<<8);
  rSDIRSP3 = data[7] | (data[6]<<8);
  rSDIRSP4 = data[9] | (data[8]<<8);
  rSDIRSP5 = data[11] | (data[10]<<8);
  rSDIRSP6 = data[13] | (data[12]<<8);
  rSDIRSP7 = data[15] | (data[14]<<8);
  rSDICmdSta |= BIT(9);
}

static void handleGPIOIPINLVL(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = rGPIOIPINLVL;
  }
}

static void handleSDICmdSta(bool isRead, uint64_t* value) {
  if(isRead) {
      *value = rSDICmdSta;
  } else {
    rSDICmdSta = CLEARBITS(rSDICmdSta, (uint16_t)(*value));
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
      // command triggered
      SDRequest command = {.cmd = cmd, .arg = arg};
      int rspLen = sd_do_command(sd, &command, rspBuffer);
      rSDICmdSta |= BIT(11);
      switch(rspLen) {
      case 0:
	rSDICmdSta |= BIT(9);
	break;
      case 4:
	shortResponse(rspBuffer);
	break;
      case 16:
	longResponse(rspBuffer);
	break;
      default:
	fprintf(stderr, "Illegal SD response length: %d\n", rspLen);
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
    rSDICmdArgH = *value;
  }
}

static void handleSDIFSTA(bool isRead, uint64_t* value) {
  if(isRead) {
    if(sd->state == sd_sendingdata_state) {
      *value = 0x1;
    } else if(sd->state == sd_receivingdata_state) {
      *value = BIT(13);
    }
  }
}

static void handleSDIDAT(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = sd_read_byte(sd);
  } else {
    sd_write_byte(sd, *value);
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

  sdBlk = blk_open("sd.img", false);
  if(sdBlk == NULL) {
    fprintf(stderr, "Error opening SD image\n");
    return 1;
  }

  sd = sd_init(sdBlk, SD_PHY_SPECv2_00_VERS);
  sd_enable(sd, true);
  
  return 0;
}
