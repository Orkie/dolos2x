#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "dolos2x.h"

static struct timeval startTime;

static uint32_t* rTCOUNT = NULL;

static void handleTimerGet(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
  printf("Handling tcount get\n");
}

int initTimer() {
  gettimeofday(&startTime, NULL);
  rTCOUNT = (uint32_t*) ((uint8_t*)getIORegs())+TCOUNT;
  
  hookRegRead(REG(TCOUNT), 4, handleTimerGet);
}
