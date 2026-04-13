#include "cpu/exec.h"
#include "cpu/rtl.h"

static inline uint8_t modrm_mod(uint8_t m) { return m >> 6; }
static inline uint8_t modrm_reg(uint8_t m) { return (m >> 3) & 0x7; }
static inline uint8_t modrm_rm (uint8_t m) { return m & 0x7; }

static inline uint8_t sib_base (uint8_t s) { return s & 0x7; }
static inline uint8_t sib_index(uint8_t s) { return (s >> 3) & 0x7; }
static inline uint8_t sib_ss   (uint8_t s) { return s >> 6; }

void load_addr(vaddr_t *eip, uint8_t mod, uint8_t rm_field, Operand *rm) {
  assert(mod != 3);

  int32_t disp = 0;
  int disp_size = 4;
  int base_reg = -1, index_reg = -1, scale = 0;
  rtl_li(&rm->addr, 0);

  if (rm_field == R_ESP) {
    uint8_t s = instr_fetch(eip, 1);
    base_reg = sib_base(s);
    scale = sib_ss(s);

    uint8_t idx = sib_index(s);
    if (idx != R_ESP) { index_reg = idx; }
  }
  else {
    /* no SIB */
    base_reg = rm_field;
  }

  if (mod == 0) {
    if (base_reg == R_EBP) { base_reg = -1; }
    else { disp_size = 0; }
  }
  else if (mod == 1) { disp_size = 1; }

  if (disp_size != 0) {
    /* has disp */
    disp = instr_fetch(eip, disp_size);
    if (disp_size == 1) { disp = (int8_t)disp; }

    rtl_addi(&rm->addr, &rm->addr, disp);
  }

  if (base_reg != -1) {
    rtl_add(&rm->addr, &rm->addr, &reg_l(base_reg));
  }

  if (index_reg != -1) {
    rtl_shli(&t0, &reg_l(index_reg), scale);
    rtl_add(&rm->addr, &rm->addr, &t0);
  }

#ifdef DEBUG
  char disp_buf[16];
  char base_buf[8];
  char index_buf[8];

  if (disp_size != 0) {
    /* has disp */
    sprintf(disp_buf, "%s%#x", (disp < 0 ? "-" : ""), (disp < 0 ? -disp : disp));
  }
  else { disp_buf[0] = '\0'; }

  if (base_reg == -1) { base_buf[0] = '\0'; }
  else { 
    sprintf(base_buf, "%%%s", reg_name(base_reg, 4));
  }

  if (index_reg == -1) { index_buf[0] = '\0'; }
  else { 
    sprintf(index_buf, ",%%%s,%d", reg_name(index_reg, 4), 1 << scale);
  }

  if (base_reg == -1 && index_reg == -1) {
    sprintf(rm->str, "%s", disp_buf);
  }
  else {
    sprintf(rm->str, "%s(%s%s)", disp_buf, base_buf, index_buf);
  }
#endif

  rm->type = OP_TYPE_MEM;
}

void read_ModR_M(vaddr_t *eip, Operand *rm, bool load_rm_val, Operand *reg, bool load_reg_val) {
  uint8_t m = instr_fetch(eip, 1);
  uint8_t mod = modrm_mod(m);
  uint8_t reg_field = modrm_reg(m);
  uint8_t rm_field = modrm_rm(m);

  decoding.ext_opcode = reg_field;
  if (reg != NULL) {
    reg->type = OP_TYPE_REG;
    reg->reg = reg_field;
    if (load_reg_val) {
      rtl_lr(&reg->val, reg->reg, reg->width);
    }

#ifdef DEBUG
    snprintf(reg->str, OP_STR_SIZE, "%%%s", reg_name(reg->reg, reg->width));
#endif
  }

  if (mod == 3) {
    rm->type = OP_TYPE_REG;
    rm->reg = rm_field;
    if (load_rm_val) {
      rtl_lr(&rm->val, rm_field, rm->width);
    }

#ifdef DEBUG
    sprintf(rm->str, "%%%s", reg_name(rm_field, rm->width));
#endif
  }
  else {
    load_addr(eip, mod, rm_field, rm);
    if (load_rm_val) {
      rtl_lm(&rm->val, &rm->addr, rm->width);
    }
  }
}
