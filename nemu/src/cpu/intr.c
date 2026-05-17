#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  assert(NO < 256);

  if (cpu.idtr_limit == 0 && cpu.idtr_base == 0) {
    panic("IDTR is not initialized when raising interrupt %u", NO);
  }

  uint32_t gate_addr = cpu.idtr_base + NO * 8;
  if (gate_addr + 7 > cpu.idtr_base + cpu.idtr_limit) {
    panic("IDT entry for vector %u out of IDTR bounds", NO);
  }

  uint32_t off_15_0 = vaddr_read(gate_addr, 2);
  uint32_t cs = vaddr_read(gate_addr + 2, 2);
  uint32_t flags = vaddr_read(gate_addr + 4, 4);
  uint32_t off_31_16 = flags >> 16;
  uint32_t present = (flags >> 15) & 1;

  if (!present) {
    panic("IDT entry for vector %u not present", NO);
  }

  uint32_t offset = (off_31_16 << 16) | off_15_0;
  rtl_li(&t0, cpu.eflags_val);
  rtl_push(&t0);
  rtl_li(&t0, cpu.cs);
  rtl_push(&t0);
  rtl_li(&t0, ret_addr);
  rtl_push(&t0);
  cpu.cs = cs;
  cpu.eip = offset;
}

void dev_raise_intr() {
  if (cpu.idtr_limit == 0 && cpu.idtr_base == 0) {
    return;
  }
  raise_intr(0x20, cpu.eip);
}
