#ifndef __DOLOS2X_H__
#define __DOLOS2X_H__
#include <unicorn/unicorn.h>
#include <SDL.h>
#include "gp2xregs.h"

#define SZ_NAND (64*1024*1024)
#define SZ_RAM (64*1024*1024)

#define CLEARBITS(in, bits) (in & (~bits))

extern int initCpus();
extern uc_engine* getArm920();
extern void* getRam();
extern void* getIORegs();
extern int mapBuffer(uint32_t address, uint32_t size, void* buf);
extern int hookRegWrite(uint32_t addr, int width, uc_cb_hookmem_t callback);
extern int hookRegRead(uint32_t addr, int width, uc_cb_hookmem_t callback);
extern int hookRegRW(uint32_t addr, int width, uc_cb_hookmem_t callback);
extern int addBreakpoint(uint32_t addr);
extern void startExecution();

extern int initNand();
extern int readBootBlock(void* buf);

extern int initTimer();
extern int initMMSP2();
extern int initNet2272(bool isF200);
extern int initVideo(SDL_Renderer* renderer);
extern int initGPIO();
extern int initUart();
extern int initSD();

#endif
