#include <stdint.h>
#include <stdbool.h>

typedef struct {
  // TODO - put here flags, registers, memory access function (deals with MMU, big/little endian) etc.
} __arm9_cpu;

typedef enum {
  COND_EQ = 0b0000,
  COND_NE = 0b0001,
  COND_CS = 0b0010,
  COND_CC = 0b0011,
  COND_MI = 0b0100,
  COND_PL = 0b0101,
  COND_VS = 0b0110,
  COND_VC = 0b0111,
  COND_HI = 0b1000,
  COND_LS = 0b1001,
  COND_GE = 0b1010,
  COND_LT = 0b1011,
  COND_GT = 0b1100,
  COND_LE = 0b1101,
  COND_AL = 0b1110
} __arm9_condition;

typedef enum {
  OP_AND = 0b0000,
  OP_SUB = 0b0010,
  OP_RSB = 0b0011,
  OP_ADD = 0b0100,
  OP_ADC = 0b0101,
  OP_SBC = 0b0110,
  OP_RSC = 0b0111,
  OP_TST = 0b1000,
  OP_TEQ = 0b1001,
  OP_CMP = 0b1010,
  OP_CMN = 0b1011,
  OP_ORR = 0b1100,
  OP_MOV = 0b1101,
  OP_BIC = 0b1110,
  OP_MVN = 0b1111
} __arm9_opcode;

typedef enum {
  INSTR_DATA_PROCESSING,
  INSTR_MULTIPLY,
  INSTR_MULTIPLY_LONG,
  INSTR_UNDEFINED
  // TODO - the rest
} __arm9_instruction_type;

typedef struct {
  uint8_t imm;
  uint8_t rotate;
} __arm9_immediate_operand;

typedef struct {
  uint8_t reg;
  uint8_t shift;
} __arm9_register_operand;

typedef struct {
  bool is_immediate;
  union Operand2 {
    __arm9_immediate_operand imm;
    __arm9_register_operand reg;
  } op;
} __arm9_operand2;

typedef struct {
  __arm9_operand2 operand2;
} __arm9_instr_data_processing;

typedef union {
  __arm9_instr_data_processing instr_data_processing;
} __arm9_instructions;

typedef struct {
  __arm9_condition cond;
  __arm9_instruction_type type;
  __arm9_instructions instr;
} __arm9_instruction;

static void arm9_decode_operand2_imm(__arm9_operand2* dest, uint16_t op) {
  dest->is_immediate = true;
  dest->op.imm.imm = op & 0xFF;
  dest->op.imm.rotate = op >> 8;
}

static void arm9_decode_operand2_reg(__arm9_operand2* dest, uint16_t op) {
  dest->is_immediate = false;
  dest->op.reg.reg = op & 0b1111;
  dest->op.reg.shift = op >> 4;
}

static bool instr_is_data_processing(uint32_t i) {
  return (i & (0x3 << 26)) == 0x0;
}

static void arm9_decode_data_processing(__arm9_instr_data_processing* dest, uint32_t i) {
  if((i & (0x1 << 25)) == 0x0) {
    arm9_decode_operand2_reg(&dest->operand2, 0x00000);
  } else {
    arm9_decode_operand2_imm(&dest->operand2, 0x00000);
  }

  // TODO
}

void arm9_decode_instruction(__arm9_instruction* dest, uint32_t i) {
  const unsigned int cond = (i&0x3C0000000) >> 24;

  if(instr_is_data_processing(i)) {
    dest->type = INSTR_DATA_PROCESSING;
    arm9_decode_data_processing(&dest->instr.instr_data_processing, i);
  } else {
    dest->type = INSTR_UNDEFINED;
  }

  dest->cond = cond;
}

