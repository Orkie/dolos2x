#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "dolos2x.h"

static SDL_Thread* uartThread;
static SDL_mutex* fifoMutex;

static volatile uint16_t rFSTATUS0 = 0x0;
static volatile uint8_t rRHB0 = 0x0;
static volatile int fifoCount = 0;
static uint8_t fifo[16];

static void handleTHB0(bool isRead, uint64_t* value) {
  if(!isRead) {
    printf("%c", (char) (*value));
    fflush(stdout);
  }
}

static void handleRHB0(bool isRead, uint64_t* value) {
  if(isRead) {
    SDL_LockMutex(fifoMutex);
    *value = fifo[--fifoCount];
    rFSTATUS0 = rFSTATUS0 - 1;
    SDL_UnlockMutex(fifoMutex);
  }
}

static void handleFSTATUS0(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = rFSTATUS0;
  }
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
	rFSTATUS0 = fifoCount;
	SDL_UnlockMutex(fifoMutex);
	break;
      }
    }    
  }
}

int initUart() {
  registerIoCallback(THB0, handleTHB0);
  registerIoCallback(RHB0, handleRHB0);
  registerIoCallback(FSTATUS0, handleFSTATUS0);
  
  fifoMutex = SDL_CreateMutex();
  uartThread = SDL_CreateThread(uartThreadFn, "UART Thread", NULL);

  return 0;
}
