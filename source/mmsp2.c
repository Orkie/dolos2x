#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dolos2x.h"

static volatile uint16_t* rFPLLVSETREG = NULL;
static volatile uint16_t* rUPLLVSETREG = NULL;
static volatile uint16_t* rAPLLVSETREG = NULL;

static void handleClockSet(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
  #ifdef DEBUG
  printf("Handling clock set: 0x%x = 0x%x\n", address, value);
  #endif

  uint16_t reg = address&0xFFFF;
  if(reg == FPLLSETVREG) {
    *rFPLLVSETREG = (uint16_t) value;
  } else if(reg == UPLLSETVREG) {
    *rUPLLVSETREG = (uint16_t) value;
  } else if(reg == APLLSETVREG) {
    *rAPLLVSETREG = (uint16_t) value;
  } else {
    fprintf(stderr, "Unknown clock set register: 0x%x\n", address);
  }
}

int initMMSP2() {
  rFPLLVSETREG = ((uint16_t*)getIORegs())+(FPLLVSETREG>>1);
  rUPLLVSETREG = ((uint16_t*)getIORegs())+(UPLLVSETREG>>1);
  rAPLLVSETREG = ((uint16_t*)getIORegs())+(APLLVSETREG>>1);

  hookRegWrite(REG(FPLLSETVREG), 2, handleClockSet);
  hookRegWrite(REG(UPLLSETVREG), 2, handleClockSet);
  hookRegWrite(REG(APLLSETVREG), 2, handleClockSet);
}
