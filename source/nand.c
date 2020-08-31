#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dolos2x.h"

#define CMD_READ 0x0
#define CMD_ERASE_SETUP 0x60
#define CMD_ERASE_CONFIRM 0xD0
#define CMD_WRITE_SETUP 0x80
#define CMD_WRITE_CONFIRM 0x10

#define BLOCK_SZ 512

static FILE* nandFp;
static void* nandRegs;

static int addrCounter = 0;
static uint32_t addrBuffer = 0x0;
static uint32_t addr = 0x0;
static uint8_t command = 0x0;
static bool commandRunning = false;
static uint8_t dataBuffer[BLOCK_SZ];
static int dataCounter = 0;

static uint16_t* rMEMNANDCTRLW = NULL;
static uint16_t* rNFDATA = NULL;

static void padFileTo(long sz) {
  fseek(nandFp, 0, SEEK_END);
  long fileSize = ftell(nandFp);
  long paddingNeeded = sz - fileSize;
  for(long i = 0 ; i < paddingNeeded ; i++) {
    fputc(0xff, nandFp);
  }
}

static void startCommand() {
  #ifdef DEBUG
  printf("Starting NAND command 0x%x with address 0x%x\n", command, addrBuffer);
  #endif

  addr = addrBuffer;
  addrBuffer = 0x0;

  switch(command) {
  case CMD_READ:
    commandRunning = true;
    dataCounter = 0;
    memset(dataBuffer, 0x0, BLOCK_SZ);
    int bytesRead;
    if(fseek(nandFp, addr, SEEK_SET) != 0 || (bytesRead = fread(dataBuffer, 1, BLOCK_SZ, nandFp)) != 512) {
      #ifdef DEBUG
      printf("Could not read full NAND block 0x%x, read %d bytes\n", addr, bytesRead);
      #endif
      return;
    }
    *rMEMNANDCTRLW |= 0x8080;
    break;
  case CMD_ERASE_CONFIRM:
    memset(dataBuffer, 0xff, BLOCK_SZ);
    uint32_t startAddress = BLOCK_SZ * addr;
    #ifdef DEBUG
    printf("Erasing block %d, addr 0x%x\n", addr, startAddress);
    #endif
    padFileTo(startAddress + BLOCK_SZ);
    fseek(nandFp, startAddress, SEEK_SET);
    fwrite(dataBuffer, 1, BLOCK_SZ, nandFp);
    fflush(nandFp);
    *rMEMNANDCTRLW |= 0x8080;
    break;
  case CMD_WRITE_SETUP:
    commandRunning = true;
    dataCounter = 0;
    memset(dataBuffer, 0x0, BLOCK_SZ);
    addrBuffer = addr;
    break;
  case CMD_WRITE_CONFIRM:
    #ifdef DEBUG
    printf("Writing to: 0x%x\n", addr);
    #endif
    padFileTo(startAddress + BLOCK_SZ);
    fseek(nandFp, addr, SEEK_SET);
    fwrite(dataBuffer, 1, BLOCK_SZ, nandFp);
    *rMEMNANDCTRLW |= 0x8080;
    break;
  default:
    fprintf(stderr, "Unknown NAND command 0x%x\n", command);
  }
}

static void handleNFCMD(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
  #ifdef DEBUG
  printf("Write to NFCMD: 0x%x\n", value);
  #endif
  command = value;
  if(command == CMD_ERASE_CONFIRM) {
    startCommand();
  } else if(command == CMD_WRITE_CONFIRM) {
    startCommand();
  }
}

static void handleNFADDR(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
  #ifdef DEBUG
  printf("Write %d to NFADDR: 0x%x (0x%x)\n", addrCounter, value, addrBuffer);
  #endif

  if(addrCounter == 0) {
    addrBuffer |= (value & 0xFF);
    addrCounter++;
  } else if(addrCounter == 1) {
    addrBuffer |= ((value & 0xFF) << (command == CMD_ERASE_SETUP ? 8 : 9));
    addrCounter++;
  } else if(addrCounter == 2) {
    addrBuffer |= ((value & 0xFF) << (command == CMD_ERASE_SETUP ? 16 : 17));
    addrCounter++;
  } else if(addrCounter == 3) {
    addrBuffer |= ((value & 0xFF) << (command == CMD_ERASE_SETUP ? 24 : 25));
    addrCounter = 0;
    if(command != CMD_ERASE_SETUP) {
      startCommand();
    }
  }
}

static void handleNFDATA(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
  #ifdef DEBUG
  printf("Access to NFDATA: 0x%x (0x%x) %d [%s]\n", address, value, size, (type == UC_MEM_READ ? "READ" : "WRITE"));
  #endif

  if(commandRunning) {
    if(type == UC_MEM_READ) {
      if(size == 2) {
	*rNFDATA = (dataBuffer[dataCounter+1] << 8) | dataBuffer[dataCounter];
	dataCounter += 2;
      } else {
	*rNFDATA = dataBuffer[dataCounter++];
      }      
    } else {
      if(size == 2) {
	dataBuffer[dataCounter++] = (value >> 8) & 0xFF;
	dataBuffer[dataCounter++] = value & 0xFF;
      } else {
	dataBuffer[dataCounter++] = value & 0xFF;	
      }      
    }

  #ifdef DEBUG
    printf("NFDATA now contains 0x%x, 0x%x in file\n", *rNFDATA, addr+dataCounter-(size == 2 ? 2 : 1));
  #endif

  } else {
    fprintf(stderr, "Tried to access NFDATA when command was not running\n");
  }
}

static void handleMEMNANDCTRLW(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
  if(type == UC_MEM_WRITE) {
    *rMEMNANDCTRLW = CLEARBITS(*rMEMNANDCTRLW, value);
  }
  #ifdef DEBUG
  printf("Write to MEMNANDCTRLW: 0x%x 0x%x\n", value, *rMEMNANDCTRLW);
  #endif
}

int initNand() {
  nandFp = fopen("nand.bin", "rb+");
  if(nandFp == NULL) {
    fprintf(stderr, "Error opening NAND image\n");
    return 1;
  }

  nandRegs = calloc(1, 4096);
  if(nandRegs == NULL) {
    fprintf(stderr, "Could not allocate NAND regs\n");
    return 2;
  }

  rMEMNANDCTRLW = (uint16_t*) ((uint8_t*)getIORegs())+MEMNANDCTRLW;
  rNFDATA = (uint16_t*) ((uint8_t*)nandRegs)+NFDATA;

  mapBuffer(NAND_BASE, 4096, nandRegs);

  hookRegWrite(NANDREG(NFCMD), 2, handleNFCMD);
  hookRegWrite(NANDREG(NFADDR), 2, handleNFADDR);
  hookRegRW(NANDREG(NFDATA), 2, handleNFDATA);
  hookRegWrite(REG(MEMNANDCTRLW), 2, handleMEMNANDCTRLW);
  
  return 0;
}

int readBootBlock(void* buf) {
  if(fread(buf, 1, 512, nandFp) != 512) {
    fprintf(stderr, "Could not read boot block from NAND\n");
    return 1;
  }

  return 0;
}
