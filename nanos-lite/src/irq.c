#include "common.h"
#include "proc.h"

_RegSet* do_syscall(_RegSet *r);

static _RegSet* do_event(_Event e, _RegSet* r) {
  switch (e.event) {
    case _EVENT_SYSCALL: return do_syscall(r);
    case _EVENT_TRAP:
      Log("Kernel trap triggered!");
      return schedule(r);
    /* Timer and other hardware IRQs use vecnull (irq = -1) for now. */
    case _EVENT_IRQ_TIME:
    case _EVENT_IRQ_IODEV:
    case _EVENT_ERROR:
      return r;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}
