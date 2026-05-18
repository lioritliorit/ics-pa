#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  return 0;
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  assert(offset >= 0 && offset + len <= sizeof(dispinfo));
  memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  assert(offset >= 0 && offset % 4 == 0 && len % 4 == 0);
  const uint32_t *pixels = buf;
  int pixel_offset = offset / 4;
  int count = len / 4;
  int width = _screen.width;
  while (count > 0) {
    int x = pixel_offset % width;
    int y = pixel_offset / width;
    int row = width - x;
    if (row > count) row = count;
    _draw_rect(pixels, x, y, row, 1);
    pixel_offset += row;
    pixels += row;
    count -= row;
  }
}

void init_device() {
  _ioe_init();
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
