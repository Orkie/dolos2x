#ifndef __DOLOS2X_H__
#define __DOLOS2X_H__
#include <unicorn/unicorn.h>
#include "gp2xregs.h"

#define SZ_NAND (64*1024*1024)
#define SZ_RAM (64*1024*1024)

#define CLEARBITS(in, bits) (in & (~bits))

#define DEBUG

extern int initCpus();
extern uc_engine* getArm920();
extern void* getRam();
extern int mapBuffer(uint32_t address, uint32_t size, void* buf);
extern int hookReg(uint32_t addr, int width, uc_cb_hookmem_t callback);
extern void startExecution();

extern int initNand();
extern int readBootBlock(void* buf);

#endif
