#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <SDL.h>
#include <termios.h>
#include <unistd.h>
#undef CTRL
#include "dolos2x.h"

static SDL_Window* sdlWindow;
static SDL_Renderer* sdlRenderer;
static struct termios old_tio, new_tio;

static void memHookCallback(uc_engine *uc, uc_mem_type type, uint64_t address, uint32_t size, int64_t value, void *user_data) {
  uint32_t pc;
  uc_reg_read(uc, UC_ARM_REG_PC, &pc);

  printf("Tried to access 0x%x 0x%x at PC = 0x%x\n", address, size, pc);
}

int init() {
  sdlWindow = SDL_CreateWindow("dolos2x", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 960, 0);
  sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);

  return 0;
}

void cleanup() {
  SDL_DestroyRenderer(sdlRenderer);
  SDL_DestroyWindow(sdlWindow);
  SDL_Quit();
  tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
  printf("\n");
}

int main(int argc, char* argv[]) {
  if(init() || initCpus()) {
    return 1;
  }

  tcgetattr(STDIN_FILENO, &old_tio);
  new_tio = old_tio;
  tcsetattr(STDIN_FILENO,TCSANOW, &old_tio);
  new_tio.c_lflag &=(~ICANON & ~ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

  atexit(cleanup);

  uc_engine* arm920 = getArm920();
  
  if(initNand() ||
     initTimer() ||
     initMMSP2() ||
     initNet2272(false) ||
     readBootBlock(getRam()) ||
     initVideo(sdlRenderer) ||
     initGPIO() ||
     initUart() ||
     initSD()
    ) {
    return 1;
  }

  #ifdef DEBUG
  //traceCode();
  #endif
  
  uc_hook memHook;
  uc_hook_add(arm920, &memHook, UC_HOOK_MEM_UNMAPPED, memHookCallback, NULL, 1, 0);

  //  addBreakpoint(0xC00082b0);
  //addBreakpoint(0xc0008284);
  
  startExecution();
  
  uint32_t pc;
  uc_reg_read(arm920, UC_ARM_REG_PC, &pc);
  
  return 0;
}
