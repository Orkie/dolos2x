#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "dolos2x.h"

static void memHookCallback(uc_engine *uc, uc_mem_type type, uint64_t address, uint32_t size, int64_t value, void *user_data) {
  uint32_t pc;
  uc_reg_read(uc, UC_ARM_REG_PC, &pc);

  printf("Tried to access 0x%x 0x%x at PC = 0x%x\n", address, size, pc);
}

void codeHookCallback(uc_engine *uc, uint64_t address, uint32_t size, void *user_data) {
  printf("Executing: 0x%x\n", address);
}

int main(int argc, char* argv[]) {
  if(initCpus()) {
    return 1;
  }

  uc_engine* arm920 = getArm920();
  
  if(initNand()) {
    return 1;
  }

  if(readBootBlock(getRam())) {
    return 1;
  }

  uc_hook cpuhook;
  uc_hook_add(arm920, &cpuhook, UC_HOOK_CODE, codeHookCallback, NULL, 1, 0);
  
  uc_hook memHook;
  uc_hook_add(arm920, &memHook, UC_HOOK_MEM_UNMAPPED, memHookCallback, NULL, 1, 0);

  addBreakpoint(0xd4);
  
  startExecution();
  
  uint32_t pc;
  uc_reg_read(arm920, UC_ARM_REG_PC, &pc);
  printf("PC is 0x%x\n", pc);
  
  return 0;
}
