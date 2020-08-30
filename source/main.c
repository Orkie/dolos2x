#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <SDL.h>
#include "dolos2x.h"

static SDL_Window* sdlWindow;
static SDL_Renderer* sdlRenderer;

static SDL_Thread *eventThread;

static void memHookCallback(uc_engine *uc, uc_mem_type type, uint64_t address, uint32_t size, int64_t value, void *user_data) {
  uint32_t pc;
  uc_reg_read(uc, UC_ARM_REG_PC, &pc);

  printf("Tried to access 0x%x 0x%x at PC = 0x%x\n", address, size, pc);
}

void codeHookCallback(uc_engine *uc, uint64_t address, uint32_t size, void *user_data) {
  printf("Executing: 0x%x\n", address);
}

int eventThreadFn(void* data) {
  SDL_Event event;
  while(true) {
    while(SDL_PollEvent(&event)) {
      if(event.type == SDL_QUIT) {
	exit(0);
      }
    }
  }
}

int init() {
  sdlWindow = SDL_CreateWindow("dolos2x", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
  sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);

  eventThread = SDL_CreateThread(eventThreadFn, "Event Thread", NULL);
  
  return 0;
}

void cleanup() {
  SDL_DestroyRenderer(sdlRenderer);
  SDL_DestroyWindow(sdlWindow);
  SDL_Quit();
}

int main(int argc, char* argv[]) {
  if(init() || initCpus()) {
    return 1;
  }
  
  uc_engine* arm920 = getArm920();
  
  if(initNand() ||
     initTimer() ||
     initMMSP2() ||
     initNet2272(false) ||
     readBootBlock(getRam()) ||
     initVideo()
    ) {
    return 1;
  }

  uc_hook cpuhook;
  uc_hook_add(arm920, &cpuhook, UC_HOOK_CODE, codeHookCallback, NULL, 1, 0);
  
  uc_hook memHook;
  uc_hook_add(arm920, &memHook, UC_HOOK_MEM_UNMAPPED, memHookCallback, NULL, 1, 0);

  atexit(cleanup);

  startExecution();
  
  uint32_t pc;
  uc_reg_read(arm920, UC_ARM_REG_PC, &pc);
  printf("PC is 0x%x\n", pc);
  
  return 0;
}
