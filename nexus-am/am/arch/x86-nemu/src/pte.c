#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*palloc_f)();
static void (*pfree_f)(void*);

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

void _pte_init(void* (*palloc)(), void (*pfree)(void*)) {
  palloc_f = palloc;
  pfree_f = pfree;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);
}

void _protect(_Protect *p) {
  PDE *updir = (PDE*)(palloc_f());
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
}

void _release(_Protect *p) {
}

void _switch(_Protect *p) {
  set_cr3(p->ptr);
}

void _map(_Protect *p, void *va, void *pa) {
  PDE *pdir = (PDE *)p->ptr;
  uint32_t pde_idx = PDX(va);
  PDE *pde = &pdir[pde_idx];
  
  if (!(*pde & PTE_P)) {
    PTE *ptab = (PTE *)palloc_f();
    *pde = (PDE)((uintptr_t)ptab | PTE_P | PTE_W | PTE_U);
  }
  
  PTE *ptab = (PTE *)PTE_ADDR(*pde);
  uint32_t pte_idx = PTX(va);
  ptab[pte_idx] = (PTE)((uintptr_t)pa | PTE_P | PTE_W | PTE_U);
}

void _unmap(_Protect *p, void *va) {
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
  uint32_t *sp = (uint32_t *)ustack.end;
  
  // 设置 _start() 的栈帧参数（都是 0 或 NULL）
  *(--sp) = 0;  // envp
  *(--sp) = 0;  // argv
  *(--sp) = 0;  // argc
  
  // 设置陷阱帧
  _RegSet *tf = (_RegSet *)((uint8_t *)sp - sizeof(_RegSet));
  
  tf->edi = 0;
  tf->esi = 0;
  tf->ebp = 0;
  tf->esp = (uint32_t)sp;  // 栈顶指向 _start() 的栈帧
  tf->ebx = 0;
  tf->edx = 0;
  tf->ecx = 0;
  tf->eax = 0;
  tf->irq = -1;
  tf->error_code = 0;
  tf->eip = (uint32_t)entry;
  tf->cs = 8;  // SEG_KCODE
  tf->eflags = 0x200;  // IF=1, 允许中断
  
  return tf;
}
