#include "common.h"
#include "proc.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  char tmp[64];
  int key = _read_key();
  const int KEYDOWN_MASK = 0x8000;
  if (key != _KEY_NONE) {
    int pressed = (key & KEYDOWN_MASK) != 0;
    int code = key & ~KEYDOWN_MASK;
    const char *name = "UNKNOWN";
    if (code >= 0 && code < (int)(sizeof(keyname)/sizeof(keyname[0])) && keyname[code]) {
      name = keyname[code];
    }
    // 检测 F12 按下，切换游戏
    if (pressed && code == _KEY_F12) {
      switch_game();
    }
    if (pressed) {
      sprintf(tmp, "kd %s\n", name);
    } else {
      sprintf(tmp, "ku %s\n", name);
    }
  } else {
    unsigned long t = _uptime();
    sprintf(tmp, "t %u\n", (unsigned int)t);
  }
  size_t slen = strlen(tmp);
  if (len < slen) slen = len;
  memcpy(buf, tmp, slen);
  return slen;
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
