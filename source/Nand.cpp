#include <stdexcept>
#include <string>

#include "dolos.h"
using namespace dolos;

Nand::Nand(Cpus* cpus) {
  nandRegs = (uint16_t*) calloc(1, 4096);
}

Nand::~Nand() {
  free(nandRegs);
}

