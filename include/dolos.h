#include <stdint.h>
#include <unicorn/unicorn.h>
#include <string>
#include <iostream>
#include <memory>
#include <fmt/core.h>

namespace dolos {

  class Logger {
  public:
    void log(std::string message) {
      std::cout << message << "\n";
    }
  };
  extern Logger* LOGGER;
  
  class Cpus {
    uc_engine* arm920;
    uc_engine* arm940;
  public:
    Cpus();
    void map(uint32_t addr, uint32_t size, void* buf);
  };

  class Peripheral {
  public:
    virtual ~Peripheral() {};
    virtual void handleChanges() {};
  };

  class Nand: public Peripheral{ 
    uint16_t* nandRegs;
  public:
    Nand(Cpus* cpus);
    ~Nand();
  };

};
