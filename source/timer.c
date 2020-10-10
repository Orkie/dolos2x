#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdatomic.h>
#include "dolos2x.h"

static struct timeval lastTickTime;

static SDL_Thread* timerThread;

static atomic_char32_t* rTCOUNT = NULL;

static void handleTimerGet(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
  /*  time_t lastSec = lastTickTime.tv_sec;
  suseconds_t lastUsec = lastTickTime.tv_usec;
  gettimeofday(&lastTickTime, NULL);

  unsigned long long elapsedNsecs = ((lastTickTime.tv_sec - lastSec) * 1000000000) + ((lastTickTime.tv_usec - lastUsec) * 1000);
  unsigned long long elapsedTicks = (elapsedNsecs / 135);
  *rTCOUNT += elapsedTicks;
  #ifdef DEBUG
  printf("Access to TCOUNT, elapsed ticks since last access %u, count is 0x%x\n", elapsedTicks, *rTCOUNT);
  #endif*/
}

static void handleTimerSet(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
  gettimeofday(&lastTickTime, NULL);
  *rTCOUNT = value;
}

int timerThreadFn(void* data) {
  while(true) {
    time_t lastSec = lastTickTime.tv_sec;
    suseconds_t lastUsec = lastTickTime.tv_usec;
    gettimeofday(&lastTickTime, NULL);

    unsigned long long elapsedNsecs = ((lastTickTime.tv_sec - lastSec) * 1000000000) + ((lastTickTime.tv_usec - lastUsec) * 1000);
    unsigned long long elapsedTicks = (elapsedNsecs / 135);
    
        atomic_fetch_add(rTCOUNT, elapsedTicks);
    //*rTCOUNT += elapsedTicks;
    usleep(2000);
  }
}

int initTimer() {
  gettimeofday(&lastTickTime, NULL);
  rTCOUNT = ((atomic_char32_t*)getIORegs())+(TCOUNT>>2);

  timerThread = SDL_CreateThread(timerThreadFn, "Timer Thread", NULL);
  
  hookRegRead(REG(TCOUNT), 4, handleTimerGet);
  hookRegWrite(REG(TCOUNT), 4, handleTimerSet);
}
