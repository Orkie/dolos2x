#include <stdio.h>
#include <stdlib.h>
#include "dolos2x.h"

static FILE* nandFp;

int initNand() {
  nandFp = fopen("nand.bin", "ab+");
  if(nandFp == NULL) {
    fprintf(stderr, "Error opening NAND image\n");
    return 1;
  }

  return 0;
}

int readBootBlock(void* buf) {
  if(fread(buf, 1, 512, nandFp) != 512) {
    return 1;
  }

  return 0;
}
