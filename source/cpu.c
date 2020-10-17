#include <stdio.h>
#include "dolos2x.h"

static uc_engine* arm920;
static void* ram;
static void* ioRegs;
static void* backupIoRegs;

extern void ioCallback(uc_engine *uc, uc_mem_type type, uint64_t address, int size, uint64_t *value, void *user_data);

// TODO handle arm940
int initCpus() {
  uc_err err = uc_open(UC_ARCH_ARM, UC_MODE_ARM | UC_MODE_ARM920, &arm920);
  if(err) {
    fprintf(stderr, "Failed on uc_open() with error returned: %u (%s)\n",
	   err, uc_strerror(err));
    return 1;
  }

  ram = calloc(1, SZ_RAM);
  if(ram == NULL) {
    fprintf(stderr, "Could not allocate 64M RAM\n");
    return 2;
  }
  mapBuffer(0x0, SZ_RAM, ram);

  ioRegs = calloc(1, 0x10000);
  if(ioRegs == NULL) {
    fprintf(stderr, "Could not allocate IO regs\n");
    return 1;
  }
  clearIoCallbacks();
  //  mapBuffer(0xC0000000, 0x10000, ioRegs);
  uc_mmio_map(arm920, 0xC0000000, 0x10000, ioCallback, NULL);

  // hack to work around the fact that unicorn doesn't take into account virtual memory when doing unmapped check,
  // Linux runs from this space
  mapBuffer(0xC0000000+0x10000, SZ_RAM, ram);
  mapBuffer(0xf0000000, 0x10000, ioRegs);
  mapBuffer(0xffff0000, SZ_RAM, ram);
    
  return 0;
}

void startExecution() {
  uc_mem_region* regions;
  int regionCount;
  uc_mem_regions(arm920, &regions, &regionCount);
  #ifdef DEBUG
  printf("%d memory regions mapped\n", regionCount);
  for(int i = 0 ; i < regionCount ; i++) {
    printf("Start = 0x%x, end = 0x%x, perms = 0x%x\n", regions[i].begin, regions[i].end, regions[i].perms);
  }
  #endif

  uc_err err = uc_emu_start(arm920, 0x0, 4294967296 - 1, 0, 0);
  if (err) {
    printf("Failed on uc_emu_start() with error returned: %u (%s)\n", err, uc_strerror(err));
  }
}

uc_engine* getArm920() {
  return arm920;
}

void* getRam() {
  return ram;
}

void* getIORegs() {
  return ioRegs;
}

int mapBuffer(uint32_t address, uint32_t size, void* buf) {
  uc_err err = uc_mem_map_ptr(arm920, address, size, UC_PROT_ALL, buf);
  if(err) {
    fprintf(stderr, "Failed to map buffer to 0x%x: %u (%s)\n",
	    address, err, uc_strerror(err));
    return 1;
  }

  return 0;
}

int hookRegWrite(uint32_t addr, int width, uc_cb_hookmem_t callback) {
  uc_hook memhook;
  #ifdef DEBUG
  printf("Hooking 0x%x\n", addr);
  #endif
  uc_hook_add(arm920, &memhook, UC_HOOK_MEM_WRITE, callback, NULL, addr, addr+(width-1));
}

int hookRegRead(uint32_t addr, int width, uc_cb_hookmem_t callback) {
  uc_hook memhook;
  #ifdef DEBUG
  printf("Hooking 0x%x\n", addr);
  #endif
  uc_hook_add(arm920, &memhook, UC_HOOK_MEM_READ, callback, NULL, addr, addr+(width-1));
}

int hookRegRW(uint32_t addr, int width, uc_cb_hookmem_t callback) {
  uc_hook memhook;
  #ifdef DEBUG
  printf("Hooking 0x%x\n", addr);
  #endif
  uc_hook_add(arm920, &memhook, UC_HOOK_MEM_READ | UC_HOOK_MEM_WRITE, callback, NULL, addr, addr+(width-1));
}

uint32_t getReg(int reg) {
  uint32_t regVal;
  uc_reg_read(arm920, reg, &regVal);
  return regVal;
}

void dumpRegs() {
  printf("  R0: %x\n", getReg(UC_ARM_REG_R0));
  printf("  R1: %x\n", getReg(UC_ARM_REG_R1));
  printf("  R2: %x\n", getReg(UC_ARM_REG_R2));
  printf("  R3: %x\n", getReg(UC_ARM_REG_R3));
  printf("  R4: %x\n", getReg(UC_ARM_REG_R4));
  printf("  R5: %x\n", getReg(UC_ARM_REG_R5));
  printf("  R6: %x\n", getReg(UC_ARM_REG_R6));
  printf("  R7: %x\n", getReg(UC_ARM_REG_R7));
  printf("  R8: %x\n", getReg(UC_ARM_REG_R8));
  printf("  R9: %x\n", getReg(UC_ARM_REG_R9));
  printf("  R10: %x\n", getReg(UC_ARM_REG_R10));
  printf("  IP: %x\n", getReg(UC_ARM_REG_IP));
  printf("  LR %x\n", getReg(UC_ARM_REG_LR));

  uint8_t* biors = (uint8_t*) backupIoRegs;
  uint8_t* iors = (uint8_t*) ioRegs;
  for(int i = 0 ; i < 0x10000 ; i++) {
    if(biors[i] != iors[i]) {
      printf("IO regs have changed: 0x%x (0x%x -> 0x%x)\n", i, biors[i], iors[i]);
    }
  }
  
  memcpy(backupIoRegs, ioRegs, 0x10000);
}

void codeHookCallback(uc_engine *uc, uint64_t address, uint32_t size, void *user_data) {
  uint32_t instr;
  uc_mem_read(uc, address, &instr, 4);
  printf("Executing: 0x%x [0x%x]\n", address, instr);
  dumpRegs();
  
  printf("----\n");
}

void memoryHookCallback(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
  printf("%s memory at 0x%x with value 0x%x\n", (type == UC_MEM_READ ? "Reading" : "Writing"), address, value);
}

void traceCode() {
  uc_hook cpuhook;
  uc_hook_add(arm920, &cpuhook, UC_HOOK_CODE, codeHookCallback, NULL, 1, 0);

  uc_hook memhook;
  uc_hook_add(arm920, &memhook, UC_HOOK_MEM_READ | UC_HOOK_MEM_WRITE, memoryHookCallback, NULL, 1, 0);
}

void breakpointCallback(uc_engine *uc, uint64_t address, uint32_t size, void *user_data) {
  uint32_t* addr = (uint32_t*) user_data;
  printf("Execution halted at breakpoint 0x%x\n", *addr);
  memcpy(backupIoRegs, ioRegs, 0x10000);
  dumpRegs();
  fflush(stdout);
  
  char c = fgetc(stdin);
  if(c == 't') {
    memset(ioRegs, 0xff, 0x10000);
    traceCode();
  }
}

int addBreakpoint(uint32_t addr) {
  uc_hook cpuhook;
  uc_hook_add(arm920, &cpuhook, UC_HOOK_CODE, breakpointCallback, &addr, addr, addr+3);
}
