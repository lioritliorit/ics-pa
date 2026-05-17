#include "common.h"

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;

#define DEFAULT_ENTRY ((void *)0x4000000)
#define RAMDISK_SIZE ((&ramdisk_end) - (&ramdisk_start))

uintptr_t loader(_Protect *as, const char *filename) {
  void *dst = DEFAULT_ENTRY;
  void *src = &ramdisk_start;
  memcpy(dst, src, RAMDISK_SIZE);
  return (uintptr_t)DEFAULT_ENTRY;
}
