#include "common.h"
#include "fs.h"
#include "syscall.h"
#include "proc.h"
#include "memory.h"

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);

  switch (a[0]) {
    case SYS_none:
      r->eax = 1;
      break;
    case SYS_open: {
      const char *pathname = (const char *)SYSCALL_ARG2(r);
      uintptr_t flags = SYSCALL_ARG3(r);
      uintptr_t mode = SYSCALL_ARG4(r);
      r->eax = fs_open(pathname, flags, mode);
      break;
    }
    case SYS_read: {
      uintptr_t fd = SYSCALL_ARG2(r);
      uintptr_t buf = SYSCALL_ARG3(r);
      uintptr_t len = SYSCALL_ARG4(r);
      r->eax = fs_read(fd, (void *)buf, len);
      break;
    }
    case SYS_write: {
      uintptr_t fd = SYSCALL_ARG2(r);
      uintptr_t buf = SYSCALL_ARG3(r);
      uintptr_t len = SYSCALL_ARG4(r);
      if (fd == 1 || fd == 2) {
        for (uintptr_t i = 0; i < len; i ++) {
          _putc(((char *)buf)[i]);
        }
        r->eax = len;
      } else {
        r->eax = fs_write(fd, (const void *)buf, len);
      }
      break;
    }
    case SYS_close: {
      uintptr_t fd = SYSCALL_ARG2(r);
      r->eax = fs_close(fd);
      break;
    }
    case SYS_lseek: {
      uintptr_t fd = SYSCALL_ARG2(r);
      off_t offset = (off_t)SYSCALL_ARG3(r);
      int whence = (int)SYSCALL_ARG4(r);
      r->eax = fs_lseek(fd, offset, whence);
      break;
    }
    case SYS_brk: {
      /* Single-task: accept any break inside the user address space. */
      uintptr_t brk = SYSCALL_ARG2(r);
      if (brk < (uintptr_t)current->as.area.start || brk >= (uintptr_t)current->as.area.end) {
        r->eax = -1;
      } else {
        mm_brk(brk);
        r->eax = 0;
      }
      break;
    }
    case SYS_exit:
      _halt(SYSCALL_ARG2(r));
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
