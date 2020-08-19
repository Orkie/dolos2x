#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "dolos2x.h"

int main(int argc, char* argv[]) {
  uc_engine* arm920;
  printf("blah1: %p\n", arm920);
  if(initArm920(arm920)) {
    fprintf(stderr, "Could not init ARM920T core\n");
    return 1;
  }
  printf("blah2: %p\n", arm920);

  if(initNand()) {
    return 1;
  }

  void* gp2xRam = calloc(1, SZ_RAM);
  if(gp2xRam == NULL) {
    fprintf(stderr, "Could not allocate 64MB for GP2X RAM\n");
    return 1;
  }

  if(readBootBlock(gp2xRam)) {
    fprintf(stderr, "Could not read boot block from NAND\n");
    return 1;
  }

  mapBuffer(arm920, 0x0, 512, gp2xRam);
  
  uc_err err = uc_emu_start(arm920, 0x0, 4294967296 - 1, 0, 0);
  if (err) {
    printf("Failed on uc_emu_start() with error returned: %u (%s)\n", err, uc_strerror(err));
  }

  uint32_t pc;
  uc_reg_read(arm920, UC_ARM_REG_PC, &pc);

  printf("PC is 0x%x\n", pc);
  
  return 0;
}
