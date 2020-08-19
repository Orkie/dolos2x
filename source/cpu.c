#include <stdio.h>
#include "dolos2x.h"

uc_engine* initArm920() {
  uc_engine *test;
  uc_err err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &test);
  if(err) {
    fprintf(stderr, "Failed on uc_open() with error returned: %u (%s)\n",
	   err, uc_strerror(err));
    return NULL;
  }

  return test;
}

int mapBuffer(uc_engine* cpu, uint32_t address, uint32_t size, void* buf) {
  uc_err err = uc_mem_map_ptr(cpu, address, size, UC_PROT_ALL, buf);
  if(err) {
    fprintf(stderr, "Failed on uc_mem_map_ptr with error returned: %u (%s)\n",
	   err, uc_strerror(err));
    return 1;
  }

  return 0;
}
