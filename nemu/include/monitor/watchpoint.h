#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

#define WP_EXPR_LEN 128

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  char expr[WP_EXPR_LEN];
  uint32_t val;
} WP;

void init_wp_pool();
WP *new_wp(const char *e);
void free_wp(int no);
void list_wp();
bool check_watchpoints();

#endif
