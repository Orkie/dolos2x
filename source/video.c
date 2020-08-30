#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <SDL.h>
#include "dolos2x.h"

static volatile uint16_t* rGPIOBPINLVL = NULL;

static uint32_t rgbFbAddr = 0x0;

static SDL_Thread* videoThread;

static SDL_Renderer* sdlRenderer;

int videoThreadFn(void* data) {
  uint32_t lastVsync = SDL_GetTicks();
  
  while(true) {
    uint32_t currentTicks = SDL_GetTicks();
    if((currentTicks - lastVsync) >= 16) {
      // TODO - figure out correct timing for v/hsync on GP2X
      lastVsync = currentTicks;
      *rGPIOBPINLVL |= BIT(4);
      SDL_Delay(40);
      *rGPIOBPINLVL = CLEARBITS(*rGPIOBPINLVL, BIT(4));

      // update screen
      uint8_t* ram = getRam();
      SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom((ram+rgbFbAddr), 320, 240, 16, 2, SDL_PIXELFORMAT_RGB565);
      SDL_Texture* texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
      SDL_RenderClear(sdlRenderer);
      SDL_RenderCopy(sdlRenderer, texture, NULL, NULL );
      SDL_RenderPresent(sdlRenderer);
      SDL_DestroyTexture(texture);
      SDL_FreeSurface(surface);
    }
    
  }

  return 0;
}

static void handleRgbFbAddrSet(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
  uint16_t reg = address & 0xFFFF;
  if(reg == MLC_STL_OADRL) {
    printf("Setting low addr: 0x%x\n", value);
    rgbFbAddr = (rgbFbAddr & 0xFFFF0000) | (value & 0xFFFF);
  } else if(reg == MLC_STL_OADRH) {
    printf("Setting high addr: 0x%x\n", value);
    rgbFbAddr = (rgbFbAddr & 0xFFFF) | ((value & 0xFFFF) << 16);
  }
}

int initVideo(SDL_Renderer* renderer) {
  sdlRenderer = renderer;
  videoThread = SDL_CreateThread(videoThreadFn, "Video Thread", NULL);

  rGPIOBPINLVL = ((uint16_t*)getIORegs())+(GPIOBPINLVL>>1);

  hookRegWrite(REG(MLC_STL_OADRL), 2, handleRgbFbAddrSet);
  hookRegWrite(REG(MLC_STL_OADRH), 2, handleRgbFbAddrSet);

  return 0;
}
