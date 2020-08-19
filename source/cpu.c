#include <stdio.h>
#include "dolos2x.h"

int initArm920(uc_engine* cpu) {
  uc_engine *test;
  printf("blll: %p\n", test);
  uc_err err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &cpu);
  printf("blll2: %p\n", test);
  if(err) {
    fprintf(stderr, "Failed on uc_open() with error returned: %u (%s)\n",
	   err, uc_strerror(err));
    return 1;
  }

  return 0;
}

int mapBuffer(uc_engine* cpu, uint32_t address, uint32_t size, void* buf) {
  uc_err err = uc_mem_map_ptr(cpu, address, size, UC_PROT_ALL, buf);
  if(err) {
    fprintf(stderr, "Failed on uc_open() with error returned: %u (%s)\n",
	   err, uc_strerror(err));
    return 1;
  }

  return 0;
}
