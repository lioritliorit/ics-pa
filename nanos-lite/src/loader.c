#include "common.h"
#include "fs.h"

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;

#define DEFAULT_ENTRY ((void *)0x4000000)

uintptr_t loader(_Protect *as, const char *filename) {
  const char *pathname = filename ? filename : "/bin/text";
  int fd = fs_open(pathname, 0, 0);
  size_t size = fs_filesz(fd);
  ssize_t nread = fs_read(fd, DEFAULT_ENTRY, size);
  assert(nread == (ssize_t)size);
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
