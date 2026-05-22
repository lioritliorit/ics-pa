#include "proc.h"
#include "memory.h"

static void *pf = NULL;

void* new_page(void) {
  assert(pf < (void *)_heap.end);
  void *p = pf;
  pf += PGSIZE;
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uint32_t new_brk) {
  uint32_t old_max = current->max_brk;
  uint32_t new_max = PGROUNDUP(new_brk);

  if (old_max == 0) {
    /* The first brk call is usually a size query from malloc. Do not
       allocate pages until the process requests more than the existing
       loaded image/bss region. */
    current->max_brk = new_max;
  } else if (new_brk > old_max) {
    uint32_t brk = PGROUNDUP(old_max);
    while (brk < new_brk) {
      void *pa = new_page();
      _map(&current->as, (void *)brk, pa);
      brk += PGSIZE;
    }
    current->max_brk = new_max;
  }
  current->cur_brk = new_brk;
  return 0;
}

void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);

  _pte_init(new_page, free_page);
}
