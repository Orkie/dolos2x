#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dolos2x.h"

static SDL_Thread *eventThread;

static volatile uint16_t* rGPIOCPINLVL = NULL;
static volatile uint16_t* rGPIODPINLVL = NULL;
static volatile uint16_t* rGPIOMPINLVL = NULL;

void toggle(volatile uint16_t* reg, int bit, bool isUp) {
  *reg = isUp ? (*reg | BIT(bit)) : (CLEARBITS(*reg, BIT(bit)));
}

void toggleKey(SDL_Keycode key, bool isUp) {
  switch(key) {
  case SDLK_UP:
    toggle(rGPIOMPINLVL, 0, isUp);
    break;
  case SDLK_LEFT:
    toggle(rGPIOMPINLVL, 2, isUp);
    break;
  case SDLK_DOWN:
    toggle(rGPIOMPINLVL, 4, isUp);
    break;
  case SDLK_RIGHT:
    toggle(rGPIOMPINLVL, 6, isUp);
    break;
  case SDLK_s: // stick
    toggle(rGPIODPINLVL, 11, isUp);
    break;
  case SDLK_LEFTBRACKET: // voldown
    toggle(rGPIODPINLVL, 6, isUp);
    break;
  case SDLK_RIGHTBRACKET: // volup
    toggle(rGPIODPINLVL, 7, isUp);
    break;
  case SDLK_l:
    toggle(rGPIOCPINLVL, 10, isUp);
    break;
  case SDLK_r:
    toggle(rGPIOCPINLVL, 11, isUp);
    break;
  case SDLK_y:
    toggle(rGPIOCPINLVL, 15, isUp);
    break;
  case SDLK_a:
    toggle(rGPIOCPINLVL, 12, isUp);
    break;
  case SDLK_b:
    toggle(rGPIOCPINLVL, 13, isUp);
    break;
  case SDLK_x:
    toggle(rGPIOCPINLVL, 14, isUp);
    break;    
  }
}

int eventThreadFn(void* data) {
  SDL_Event event;
  while(true) {
    while(SDL_PollEvent(&event)) {
      if(event.type == SDL_QUIT) {
	exit(0);
      } else if(event.type == SDL_KEYUP) {
	toggleKey(event.key.keysym.sym, true);
      } else if(event.type == SDL_KEYDOWN) {
	toggleKey(event.key.keysym.sym, false);	
      }
    }
  }
}

int initGPIO() {
  rGPIOCPINLVL = ((uint16_t*)getIORegs())+(GPIOCPINLVL>>1);
  rGPIODPINLVL = ((uint16_t*)getIORegs())+(GPIODPINLVL>>1);
  rGPIOMPINLVL = ((uint16_t*)getIORegs())+(GPIOMPINLVL>>1);

  *rGPIOCPINLVL = 0xffff;
  *rGPIODPINLVL = 0xffff;
  *rGPIOMPINLVL = 0xffff;

  eventThread = SDL_CreateThread(eventThreadFn, "Event Thread", NULL);

  return 0;
}
