#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
#define KBD_DATA_PORT 0x60
#define KBD_STAT_PORT 0x64
#define KBD_HASKEY_MASK 0x1
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  return inl(RTC_PORT) - boot_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  int j;
  for (j = 0; j < h; j++) {
    memcpy(fb + (y + j) * _screen.width + x, pixels + j * w, w * sizeof(uint32_t));
  }
}

void _draw_sync() {
}

int _read_key() {
  if ((inb(KBD_STAT_PORT) & KBD_HASKEY_MASK) == 0) {
    return _KEY_NONE;
  }
  return inl(KBD_DATA_PORT);
}
