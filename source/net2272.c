#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dolos2x.h"

void* net2272Regs = NULL;

int initNet2272(bool isF200) {
  net2272Regs = calloc(1, 4096);
  if(net2272Regs == NULL) {
    fprintf(stderr, "Could not allocate NET2272 regs\n");
    return 1;
  }

  memset(net2272Regs, isF200 ? 0xFF : 0x00, 4096);
  mapBuffer(0x88000000, 4096, net2272Regs);

  return 0;
}
