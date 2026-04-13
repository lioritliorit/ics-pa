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
#ifdef DEBUG
  if (decoding.jmp_eip == 0x00100268) {
    uint32_t a0 = vaddr_read(cpu.esp, 4);
    uint32_t a1 = vaddr_read(cpu.esp + 4, 4);
    Log("call strcmp pre-push: esp=0x%08x top=0x%08x next=0x%08x", cpu.esp, a0, a1);
  }
#endif
  cpu.esp -= width;
  vaddr_write(cpu.esp, width, t0);
  decoding.is_jmp = 1;

#ifdef DEBUG
  Log("call width=%d target=0x%08x seq_eip=0x%08x esp=0x%08x", width, decoding.jmp_eip, decoding.seq_eip, cpu.esp);
#endif
  print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(ret) {
  int width = decoding.is_operand_size_16 ? 2 : 4;
  t0 = vaddr_read(cpu.esp, width);
  cpu.esp += width;
  decoding.jmp_eip = (width == 2) ? (t0 & 0xffff) : t0;
  decoding.is_jmp = 1;

#ifdef DEBUG
  Log("ret pop width=%d -> jmp_eip=0x%08x esp=0x%08x eax=0x%08x",
      width, decoding.jmp_eip, cpu.esp, cpu.eax);
#endif
  print_asm("ret");
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
