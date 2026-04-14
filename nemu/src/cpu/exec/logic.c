#include "cpu/exec.h"

make_EHelper(test) {
  rtl_and(&t2, &id_dest->val, &id_src->val);

  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_li(&t0, 0);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);

  print_asm_template2(test);
}

make_EHelper(and) {
  // Ensure id_dest->val is loaded
  if (id_dest->type == OP_TYPE_REG) {
    rtl_lr(&id_dest->val, id_dest->reg, id_dest->width);
  }
  
  rtl_and(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_li(&t0, 0);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);

  print_asm_template2(and);
}

make_EHelper(xor) {
  // Ensure id_dest->val is loaded
  if (id_dest->type == OP_TYPE_REG) {
    rtl_lr(&id_dest->val, id_dest->reg, id_dest->width);
  }
  
  rtl_xor(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_li(&t0, 0);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);

  print_asm_template2(xor);
}

make_EHelper(or) {
  // Ensure id_dest->val is loaded
  if (id_dest->type == OP_TYPE_REG) {
    rtl_lr(&id_dest->val, id_dest->reg, id_dest->width);
  }
  
  rtl_or(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_li(&t0, 0);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);

  print_asm_template2(or);
}

make_EHelper(rol) {
  if (id_dest->type == OP_TYPE_REG) {
    rtl_lr(&id_dest->val, id_dest->reg, id_dest->width);
  }

  uint32_t bits = id_dest->width * 8;
  uint32_t mask = (id_dest->width == 4) ? 0xffffffffu : ((1u << bits) - 1);
  uint32_t count = id_src->val & 0x1f;
  count %= bits;

  uint32_t val = id_dest->val & mask;
  uint32_t res = val;
  if (count != 0) {
    res = ((val << count) | (val >> (bits - count))) & mask;

    rtl_li(&t0, res & 1);
    rtl_set_CF(&t0);

    if (count == 1) {
      uint32_t msb = (res >> (bits - 1)) & 1;
      rtl_li(&t0, msb ^ (res & 1));
      rtl_set_OF(&t0);
    }
  }

  rtl_li(&t2, res);
  operand_write(id_dest, &t2);
  print_asm_template2(rol);
}

make_EHelper(sar) {
  // Ensure id_dest->val is loaded
  if (id_dest->type == OP_TYPE_REG) {
    rtl_lr(&id_dest->val, id_dest->reg, id_dest->width);
  }
  
  rtl_mv(&t1, &id_dest->val);  // Save original value for CF calculation
  rtl_sar(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);
  // CF = LSB of original value if shift count > 0
  rtl_li(&t0, 0);
  rtl_ne(&t3, &id_src->val, &t0);
  rtl_andi(&t0, &t1, 1);
  rtl_and(&t0, &t0, &t3);
  rtl_set_CF(&t0);
  // OF is undefined for shifts, but we set it to 0
  rtl_li(&t0, 0);
  rtl_set_OF(&t0);

  print_asm_template2(sar);
}

make_EHelper(shl) {
  // Ensure id_dest->val is loaded
  if (id_dest->type == OP_TYPE_REG) {
    rtl_lr(&id_dest->val, id_dest->reg, id_dest->width);
  }
  
  rtl_mv(&t1, &id_dest->val);  // Save original value for CF calculation
  rtl_shl(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);
  // CF = MSB of original value if shift count > 0
  rtl_li(&t0, 0);
  rtl_ne(&t3, &id_src->val, &t0);
  if (id_dest->width == 4) {
    rtl_msb(&t0, &t1, 4);
  } else if (id_dest->width == 2) {
    rtl_msb(&t0, &t1, 2);
  } else {
    rtl_msb(&t0, &t1, 1);
  }
  rtl_and(&t0, &t0, &t3);
  rtl_set_CF(&t0);
  // OF is undefined for shifts, but we set it to 0
  rtl_li(&t0, 0);
  rtl_set_OF(&t0);

  print_asm_template2(shl);
}

make_EHelper(shr) {
  // Ensure id_dest->val is loaded
  if (id_dest->type == OP_TYPE_REG) {
    rtl_lr(&id_dest->val, id_dest->reg, id_dest->width);
  }
  
  rtl_mv(&t1, &id_dest->val);  // Save original value for CF calculation
  rtl_shr(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);
  // CF = LSB of original value if shift count > 0
  rtl_li(&t0, 0);
  rtl_ne(&t3, &id_src->val, &t0);
  rtl_andi(&t0, &t1, 1);
  rtl_and(&t0, &t0, &t3);
  rtl_set_CF(&t0);
  // OF is undefined for shifts, but we set it to 0
  rtl_li(&t0, 0);
  rtl_set_OF(&t0);

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  // Ensure id_dest->val is loaded
  if (id_dest->type == OP_TYPE_REG) {
    rtl_lr(&id_dest->val, id_dest->reg, id_dest->width);
  }
  
  rtl_not(&t2, &id_dest->val);
  operand_write(id_dest, &t2);

  print_asm_template1(not);
}
