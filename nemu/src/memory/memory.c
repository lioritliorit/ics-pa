#include "nemu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  Assert(addr + len <= PMEM_SIZE,
      "physical address(0x%08x) is out of bound (eip=0x%08x)", addr, cpu.eip);
  uint32_t ret = 0;
  memcpy(&ret, guest_to_host(addr), len);
  return ret;
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  Assert(addr + len <= PMEM_SIZE,
      "physical address(0x%08x) is out of bound (eip=0x%08x)", addr, cpu.eip);
  memcpy(guest_to_host(addr), &data, len);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  paddr_write(addr, len, data);
}
