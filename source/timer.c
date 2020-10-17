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

int timerThreadFn(void* data) {
  while(true) {
    time_t lastSec = lastTickTime.tv_sec;
    suseconds_t lastUsec = lastTickTime.tv_usec;
    gettimeofday(&lastTickTime, NULL);

    unsigned long long elapsedNsecs = ((lastTickTime.tv_sec - lastSec) * 1000000000) + ((lastTickTime.tv_usec - lastUsec) * 1000);
    unsigned long long elapsedTicks = (elapsedNsecs / 135);
    
    atomic_fetch_add(rTCOUNT, elapsedTicks);
    usleep(2000);
  }
}

static void handleTCOUNT(bool isRead, uint64_t* value) {
  if(isRead) {
    *value = atomic_load(rTCOUNT);
  } else {
    atomic_store(rTCOUNT, *value);
  }
}

int initTimer() {
  gettimeofday(&lastTickTime, NULL);
  rTCOUNT = ((atomic_char32_t*)getIORegs())+(TCOUNT>>2);

  timerThread = SDL_CreateThread(timerThreadFn, "Timer Thread", NULL);

  registerIoCallback(TCOUNT, handleTCOUNT);

  return 0;
}
