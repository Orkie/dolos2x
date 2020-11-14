// TODO - this uses a lot of memory for a very sparsely populated jump table - should swap in a proper hash table implementation sometime
#include "dolos2x.h"

#define MAX_REGS 0x10000

static mmio_access_t ioCallbacks[MAX_REGS];

void ioCallback(uc_engine *uc, uc_mem_type type, uint64_t address, int size, uint64_t *value, void *user_data) {
  if(address >= MAX_REGS) {
    printf("Illegal IO register... 0x%x\n", address);
    return;
  }

  mmio_access_t callback = ioCallbacks[address];
  if(callback == NULL) {
    printf("Unhandled IO register callback... %s address[0x%x], size[%d], value[0x%x]\n", (type == UC_MEM_READ ? "READ" : type == UC_MEM_WRITE ? "WRITE" : "UKNOWN ACCESS"), address, size, value);
    return;
  }

  (*callback)(type == UC_MEM_READ, value);
}

void clearIoCallbacks() {
  memset(ioCallbacks, 0x0, sizeof(mmio_access_t*)*MAX_REGS);
}

void registerIoCallback(uint32_t address, mmio_access_t cb) {
  if(address < MAX_REGS) {
    ioCallbacks[address] = cb;
  } else {
    printf("Could not register IO callback for addresss 0x%x\n", address);
  }
}
