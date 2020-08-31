#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "dolos2x.h"

static SDL_Thread* uartThread;
static SDL_mutex* fifoMutex;

static volatile uint16_t* rFSTATUS0 = NULL;
static volatile uint8_t* rRHB0 = NULL;
static volatile int fifoCount = 0;
static uint8_t fifo[16];

static void handleUartPut(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
  printf("%c", (char) value);
  fflush(stdout);
}

static void handleUartGet(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
  SDL_LockMutex(fifoMutex);
  *rRHB0 = fifo[--fifoCount];
  *rFSTATUS0 = *rFSTATUS0 - 1;
  SDL_UnlockMutex(fifoMutex);
}

int uartThreadFn(void* data) {
  while(true) {
    int c = fgetc(stdin);
    while(true) {
      SDL_LockMutex(fifoMutex);
      if(fifoCount > 15) {
	SDL_UnlockMutex(fifoMutex);
	continue;
      } else {
	fifo[fifoCount++] = (char) c;
	*rFSTATUS0 = fifoCount;
	SDL_UnlockMutex(fifoMutex);
	break;
      }
    }    
  }
}

int initUart() {
  hookRegWrite(REG(THB0), 1, handleUartPut);
  hookRegRead(REG(RHB0), 1, handleUartGet);

  rFSTATUS0 = ((uint16_t*)getIORegs())+(FSTATUS0>>1);
  rRHB0 = ((uint8_t*)getIORegs())+(RHB0);

  fifoMutex = SDL_CreateMutex();
  uartThread = SDL_CreateThread(uartThreadFn, "UART Thread", NULL);

  return 0;
}
