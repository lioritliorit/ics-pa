#include "common.h"
#include "fs.h"
#include "proc.h"
#include "memory.h"

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;

#define DEFAULT_ENTRY ((void *)0x8048000)

uintptr_t loader(_Protect *as, const char *filename) {
  const char *pathname = filename ? filename : "/bin/text";
  int fd = fs_open(pathname, 0, 0);
  size_t size = fs_filesz(fd);
  
  void *va = DEFAULT_ENTRY;
  size_t remain = size;
  while (remain > 0) {
    void *pa = new_page();
    _map(as, va, pa);
    size_t len = (remain > PGSIZE) ? PGSIZE : remain;
    ssize_t nread = fs_read(fd, pa, len);
    assert(nread == (ssize_t)len);
    va += PGSIZE;
    remain -= len;
  }
  
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
