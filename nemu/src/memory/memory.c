#include "nemu.h"
#include "memory/mmu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int map_no = is_mmio(addr);
  if (map_no != -1) {
    return mmio_read(addr, len, map_no);
  }

  Assert(addr + len <= PMEM_SIZE,
      "physical address(0x%08x) is out of bound (eip=0x%08x)", addr, cpu.eip);
  uint32_t ret = 0;
  memcpy(&ret, guest_to_host(addr), len);
  return ret;
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int map_no = is_mmio(addr);
  if (map_no != -1) {
    mmio_write(addr, len, data, map_no);
    return;
  }

  Assert(addr + len <= PMEM_SIZE,
      "physical address(0x%08x) is out of bound (eip=0x%08x)", addr, cpu.eip);
  memcpy(guest_to_host(addr), &data, len);
}

static paddr_t page_translate(vaddr_t addr, bool is_write) {
  if (!(cpu.cr0 & 0x80000000)) {
    return addr;
  }

  uint32_t pde_addr = ((cpu.cr3 & 0xfffff000) | ((addr >> 22) << 2));
  uint32_t pde_val = paddr_read(pde_addr, 4);
  assert(pde_val & 0x1);

  pde_val |= 0x20;
  paddr_write(pde_addr, 4, pde_val);

  uint32_t pte_addr = ((pde_val & 0xfffff000) | (((addr >> 12) & 0x3ff) << 2));
  uint32_t pte_val = paddr_read(pte_addr, 4);
  assert(pte_val & 0x1);

  pte_val |= 0x20;
  if (is_write) {
    pte_val |= 0x40;
  }
  paddr_write(pte_addr, 4, pte_val);

  return (pte_val & 0xfffff000) | (addr & 0xfff);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if (cpu.cr0 & 0x80000000) {
    if ((addr & 0xfff) + len > PAGE_SIZE) {
      uint32_t result = 0;
      int len1 = PAGE_SIZE - (addr & 0xfff);
      int len2 = len - len1;
      paddr_t paddr1 = page_translate(addr, false);
      paddr_t paddr2 = page_translate(addr + len1, false);
      uint32_t val1 = paddr_read(paddr1, len1);
      uint32_t val2 = paddr_read(paddr2, len2);
      memcpy(&result, &val1, len1);
      memcpy(((uint8_t*)&result) + len1, &val2, len2);
      return result;
    }
    paddr_t paddr = page_translate(addr, false);
    return paddr_read(paddr, len);
  } else {
    return paddr_read(addr, len);
  }
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if (cpu.cr0 & 0x80000000) {
    if ((addr & 0xfff) + len > PAGE_SIZE) {
      int len1 = PAGE_SIZE - (addr & 0xfff);
      int len2 = len - len1;
      paddr_t paddr1 = page_translate(addr, true);
      paddr_t paddr2 = page_translate(addr + len1, true);
      uint32_t val1 = 0, val2 = 0;
      memcpy(&val1, &data, len1);
      memcpy(&val2, ((uint8_t*)&data) + len1, len2);
      paddr_write(paddr1, len1, val1);
      paddr_write(paddr2, len2, val2);
      return;
    }
    paddr_t paddr = page_translate(addr, true);
    paddr_write(paddr, len, data);
  } else {
    paddr_write(addr, len, data);
  }
}
