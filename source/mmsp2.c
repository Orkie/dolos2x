#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dolos2x.h"

static volatile uint16_t rFPLLVSETREG = 0x0;
static volatile uint16_t rUPLLVSETREG = 0x0;
static volatile uint16_t rAPLLVSETREG = 0x0;

static void handleFPLLSETVREG(bool isRead, uint64_t* value) {
  if(!isRead) {
    rFPLLVSETREG = *value;
  }
}

static void handleFPLLVSETREG(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = rFPLLVSETREG;
  }
}

static void handleUPLLSETVREG(bool isRead, uint64_t* value) {
  if(!isRead) {
    rUPLLVSETREG = *value;
  }
}

static void handleUPLLVSETREG(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = rUPLLVSETREG;
  }
}

static void handleAPLLSETVREG(bool isRead, uint64_t* value) {
  if(!isRead) {
    rAPLLVSETREG = *value;
  }
}

static void handleAPLLVSETREG(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = rAPLLVSETREG;
  }
}

int initMMSP2() {
  registerIoCallback(FPLLSETVREG, handleFPLLSETVREG);
  registerIoCallback(FPLLVSETREG, handleFPLLVSETREG);
  registerIoCallback(UPLLSETVREG, handleUPLLSETVREG);
  registerIoCallback(UPLLVSETREG, handleUPLLVSETREG);
  registerIoCallback(APLLSETVREG, handleAPLLSETVREG);
  registerIoCallback(APLLVSETREG, handleAPLLVSETREG);

  return 0;
}
