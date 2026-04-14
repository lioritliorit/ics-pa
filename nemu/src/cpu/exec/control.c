#include "cpu/exec.h"

make_EHelper(jmp) {
  // the target address is calculated at the decode stage
  decoding.is_jmp = 1;

  print_asm("jmp %x", decoding.jmp_eip);
}

make_EHelper(jcc) {
  // the target address is calculated at the decode stage
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  decoding.is_jmp = t2;

  print_asm("j%s %x", get_cc_name(subcode), decoding.jmp_eip);
}

make_EHelper(loop) {
  // 0xe0/0xe1/0xe2: loopne/loope/loop rel8
  // This PA only needs 32-bit address-size behavior (uses ECX).
  cpu.ecx --;
  uint8_t subcode = decoding.opcode & 0x3;

  bool take = false;
  if (subcode == 0x2) { // loop
    take = (cpu.ecx != 0);
  } else if (subcode == 0x0) { // loopne/loopnz
    take = (cpu.ecx != 0) && (cpu.eflags.ZF == 0);
  } else { // loope/loopz
    take = (cpu.ecx != 0) && (cpu.eflags.ZF == 1);
  }

  decoding.is_jmp = take;
  print_asm("loop%s %x", subcode == 0x0 ? "ne" : (subcode == 0x1 ? "e" : ""), decoding.jmp_eip);
}

make_EHelper(jcxz) {
  // 0xe3: jcxz/jecxz rel8 (use ECX in 32-bit mode)
  decoding.is_jmp = (cpu.ecx == 0);
  print_asm("jecxz %x", decoding.jmp_eip);
}

make_EHelper(jmp_rm) {
  decoding.jmp_eip = id_dest->val;
  decoding.is_jmp = 1;

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {
  // the target address is calculated at the decode stage
  int width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_li(&t0, decoding.seq_eip);
  if (width == 2) { t0 &= 0xffff; }
  cpu.esp -= width;
  vaddr_write(cpu.esp, width, t0);
  decoding.is_jmp = 1;

  print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(ret) {
  int width = decoding.is_operand_size_16 ? 2 : 4;
  t0 = vaddr_read(cpu.esp, width);
  cpu.esp += width;
  decoding.jmp_eip = (width == 2) ? (t0 & 0xffff) : t0;
  decoding.is_jmp = 1;

  print_asm("ret");
}

make_EHelper(ret_imm) {
  int width = decoding.is_operand_size_16 ? 2 : 4;
  t0 = vaddr_read(cpu.esp, width);
  cpu.esp += width;
  cpu.esp += (id_dest->imm & 0xffff);
  decoding.jmp_eip = (width == 2) ? (t0 & 0xffff) : t0;
  decoding.is_jmp = 1;

  print_asm("ret $0x%x", id_dest->imm & 0xffff);
}

make_EHelper(call_rm) {
  int width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_li(&t0, decoding.seq_eip);
  if (width == 2) { t0 &= 0xffff; }
  cpu.esp -= width;
  vaddr_write(cpu.esp, width, t0);
  decoding.jmp_eip = id_dest->val;
  decoding.is_jmp = 1;

  print_asm("call *%s", id_dest->str);
}
