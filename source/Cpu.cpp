#include <stdexcept>
#include <string>

#include "dolos.h"
using namespace dolos;

Cpus::Cpus() {
  uc_err arm920Err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &arm920);
  if(arm920Err) {
    throw std::runtime_error(std::string("Could not init ARM920T core: ") + uc_strerror(arm920Err));
  }

  uc_err arm940Err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &arm940);
  if(arm940Err) {
    throw std::runtime_error(std::string("Could not init ARM940T core: ") + uc_strerror(arm940Err));
  }
}

  // TODO - we need to keep track of the arm940 bank and also map into its address space. We may need to remap when the bank changes? In which case we'll need to get the memory regions for the arm920, then reapply them to the arm940 - this means having an explicit callback hook into the 940 bank register
void Cpus::map(uint32_t addr, uint32_t size, void* buf) {
  uc_err err = uc_mem_map_ptr(arm920, addr, size, UC_PROT_ALL, buf);
  if(err) {
    throw std::runtime_error(fmt::format("Failed to map {x}: {}", addr, uc_strerror(err)));
  }
  LOGGER->log(fmt::format("Successfully mapped 0x{x}", addr));
}
