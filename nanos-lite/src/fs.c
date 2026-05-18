#include "fs.h"

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
void ramdisk_read(void *buf, off_t offset, size_t len);
void ramdisk_write(const void *buf, off_t offset, size_t len);

size_t events_read(void *buf, size_t len);
void dispinfo_read(void *buf, off_t offset, size_t len);
void fb_write(const void *buf, off_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

static Finfo *fs_getfile(int fd) {
  assert(fd >= 0 && fd < NR_FILES);
  return &file_table[fd];
}

void init_fs() {
  file_table[FD_FB].size = _screen.width * _screen.height * 4;
}

int fs_open(const char *pathname, int flags, int mode) {
  for (int i = 0; i < NR_FILES; i ++) {
    if (strcmp(file_table[i].name, pathname) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  assert(0);
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
  if (fd == FD_STDIN) {
    return 0;
  }
  Finfo *f = fs_getfile(fd);
  if (fd == FD_EVENTS) {
    return events_read(buf, len);
  }
  if (fd == FD_DISPINFO) {
    size_t remain = f->size - f->open_offset;
    if (len > remain) len = remain;
    dispinfo_read(buf, f->open_offset, len);
    f->open_offset += len;
    return len;
  }
  if (fd == FD_FB) {
    return 0;
  }
  size_t remain = f->size - f->open_offset;
  if (remain == 0) return 0;
  if (len > remain) len = remain;
  ramdisk_read(buf, f->disk_offset + f->open_offset, len);
  f->open_offset += len;
  return len;
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
  if (fd == FD_STDOUT || fd == FD_STDERR) {
    for (size_t i = 0; i < len; i ++) {
      _putc(((const char *)buf)[i]);
    }
    return len;
  }
  if (fd == FD_STDIN) {
    return 0;
  }
  Finfo *f = fs_getfile(fd);
  if (fd == FD_FB) {
    size_t remain = f->size - f->open_offset;
    if (len > remain) len = remain;
    fb_write(buf, f->open_offset, len);
    f->open_offset += len;
    return len;
  }
  if (fd == FD_EVENTS || fd == FD_DISPINFO) {
    return 0;
  }
  size_t remain = f->size - f->open_offset;
  if (remain == 0) return 0;
  if (len > remain) len = remain;
  ramdisk_write(buf, f->disk_offset + f->open_offset, len);
  f->open_offset += len;
  return len;
}

int fs_close(int fd) {
  Finfo *f = fs_getfile(fd);
  f->open_offset = 0;
  return 0;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  if (fd == FD_STDIN || fd == FD_STDOUT || fd == FD_STDERR) {
    return 0;
  }
  Finfo *f = fs_getfile(fd);
  off_t new_offset = 0;
  if (whence == SEEK_SET) {
    new_offset = offset;
  } else if (whence == SEEK_CUR) {
    new_offset = f->open_offset + offset;
  } else if (whence == SEEK_END) {
    new_offset = f->size + offset;
  } else {
    assert(0);
  }
  if (new_offset < 0) new_offset = 0;
  if (new_offset > (off_t)f->size) new_offset = f->size;
  f->open_offset = new_offset;
  return new_offset;
}

size_t fs_filesz(int fd) {
  return fs_getfile(fd)->size;
}
