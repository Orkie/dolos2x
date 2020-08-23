#include <stdio.h>
#include <stdlib.h>
#include "dolos2x.h"

static FILE* nandFp;
static void* nandRegs;

static void handleNFCMD(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
  printf("NFCMD HAS BEEN WRITTEN TO!!!: 0x%x\n", value);
}

static void handleMEMNANDCTRLW(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
  printf("MEMNANDCTRLW HAS BEEN WRITTEN TO!!!: 0x%x\n", value);
}

int initNand(uc_engine* cpu) {
  nandFp = fopen("nand.bin", "ab+");
  if(nandFp == NULL) {
    fprintf(stderr, "Error opening NAND image\n");
    return 1;
  }

  nandRegs = calloc(1, 4096);
  if(nandRegs == NULL) {
    fprintf(stderr, "Could not allocate NAND regs\n");
    return 2;
  }

  mapBuffer(0x9C000000, 4096, nandRegs);

  hookReg(0x9C000000, 2, handleNFCMD);
  hookReg(REG(MEMNANDCTRLW), 2, handleNFCMD);
  
  return 0;
}

int readBootBlock(void* buf) {
  if(fread(buf, 1, 512, nandFp) != 512) {
    fprintf(stderr, "Could not read boot block from NAND\n");
    return 1;
  }

  return 0;
}
