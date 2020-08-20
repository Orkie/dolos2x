#ifndef __DOLOS2X_H__
#define __DOLOS2X_H__
#include <unicorn/unicorn.h>

#define SZ_NAND (64*1024*1024)
#define SZ_RAM (64*1024*1024)

typedef void (*MMSP2Peripheral)();

extern uc_engine* initArm920();
int mapBuffer(uc_engine* cpu, uint32_t address, uint32_t size, void* buf);

int initNand();
int readBootBlock(void* buf);

#endif
