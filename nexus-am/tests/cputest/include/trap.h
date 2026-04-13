#ifndef __TRAP_H__
#define __TRAP_H__

#include <am.h>
#include <klib.h>

#define nemu_assert(cond) do { \
  if (!(cond)) { \
    unsigned int _line = (0x10000u | (unsigned int)__LINE__); \
    asm volatile( \
      "movl %%eax, %%ebx; \n" \
      "movl %0, %%eax; \n" \
      ".byte 0xd6      \n" \
      : \
      : "r"(_line) \
      : "eax", "ebx" \
    ); \
    while (1); \
  } \
} while (0)

#endif
