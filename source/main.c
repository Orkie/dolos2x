#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "dolos2x.h"

static void memHookCallback(uc_engine *uc, uc_mem_type type, uint64_t address, uint32_t size, int64_t value, void *user_data) {
  uint32_t pc;
  uc_reg_read(uc, UC_ARM_REG_PC, &pc);

  printf("Tried to access 0x%x 0x%x at PC = 0x%x\n", address, size, pc);
}

int main(int argc, char* argv[]) {
  uc_engine* arm920 = NULL;
  if((arm920 = initArm920()) == NULL) {
    fprintf(stderr, "Could not init ARM920T core\n");
    return 1;
  }

  if(initNand()) {
    return 1;
  }

  void* gp2xRam = calloc(1, SZ_RAM);
  if(gp2xRam == NULL) {
    fprintf(stderr, "Could not allocate 64MB for GP2X RAM\n");
    return 1;
  }

  void* ioRegs = calloc(1, 0x10000);
  if(ioRegs == NULL) {
    fprintf(stderr, "Could not allocate IO regs\n");
    return 1;
  }

  void* nandRegs = calloc(1, 4096);
  if(nandRegs == NULL) {
    fprintf(stderr, "Could not allocate NAND regs\n");
    return 1;
  }

  if(readBootBlock(gp2xRam)) {
    fprintf(stderr, "Could not read boot block from NAND\n");
    return 1;
  }

  mapBuffer(arm920, 0x0, SZ_RAM, gp2xRam);
  mapBuffer(arm920, 0xC0000000, 0x10000, ioRegs);
  mapBuffer(arm920, 0x9C000000, 4096, nandRegs);

  uint32_t buf[512/4];
  uc_err rerr = uc_mem_read(arm920, 0xc0000000, buf, 512);
  printf("rerr: %d\n", rerr);
  if(!rerr) {
    for(int i = 0 ; i < (512/4) ; i++) {
      printf("0x%x: 0x%x\n", i*4, buf[i]);
    }
  }

  buf[0] = 0x12345678;
  rerr = uc_mem_write(arm920, 0xc0000000, buf, 512);
  printf("rerr: %d\n", rerr);
rerr = uc_mem_read(arm920, 0xc0000000, buf, 512);
  printf("rerr: %d\n", rerr);
  if(!rerr) {
    for(int i = 0 ; i < (512/4) ; i++) {
      printf("0x%x: 0x%x\n", i*4, buf[i]);
    }
  }
  
  uc_mem_region* regions;
  int regionCount;
  uc_mem_regions(arm920, &regions, &regionCount);
  printf("%d memory regions mapped\n", regionCount);
  for(int i = 0 ; i < regionCount ; i++) {
    printf("Start = 0x%x, end = 0x%x, perms = 0x%x\n", regions[i].begin, regions[i].end, regions[i].perms);
  }

  uc_hook memHook;
  uc_hook_add(arm920, &memHook, UC_HOOK_MEM_UNMAPPED, memHookCallback, NULL, 1, 0);
  
  uc_err err = uc_emu_start(arm920, 0x0, 4294967296 - 1, 0, 0);
  if (err) {
    printf("Failed on uc_emu_start() with error returned: %u (%s)\n", err, uc_strerror(err));
  }

  uint32_t pc;
  uc_reg_read(arm920, UC_ARM_REG_PC, &pc);

  printf("PC is 0x%x\n", pc);
  
  return 0;
}
