#include "nemu.h"
#include "monitor/monitor.h"
#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

WP *new_wp(const char *e) {
  if (free_ == NULL) {
    printf("No free watchpoint.");
    return NULL;
  }

  WP *wp = free_;
  free_ = free_->next;

  wp->next = head;
  head = wp;

  strncpy(wp->expr, e, WP_EXPR_LEN - 1);
  wp->expr[WP_EXPR_LEN - 1] = '\0';

  bool success;
  wp->val = expr(wp->expr, &success);
  if (!success) {
    printf("Invalid expression: %s\n", wp->expr);
    free_wp(wp->NO);
    return NULL;
  }

  return wp;
}

void free_wp(int no) {
  WP *p = head, *prev = NULL;
  while (p) {
    if (p->NO == no) {
      if (prev) {
        prev->next = p->next;
      } else {
        head = p->next;
      }
      p->next = free_;
      free_ = p;
      return;
    }
    prev = p;
    p = p->next;
  }
  printf("No watchpoint number %d\n", no);
}

void list_wp() {
  WP *p = head;
  if (!p) {
    printf("No watchpoints\n");
    return;
  }

  while (p) {
    printf("[%d] %s = 0x%08x\n", p->NO, p->expr, p->val);
    p = p->next;
  }
}

bool check_watchpoints() {
  WP *p = head;
  bool triggered = false;
  while (p) {
    bool success;
    uint32_t new_val = expr(p->expr, &success);
    if (!success) {
      /* Ignore evaluation errors during watchpoint checks. */
      p = p->next;
      continue;
    }
    if (new_val != p->val) {
      printf("Watchpoint %d triggered: %s\n", p->NO, p->expr);
      printf("  old value = 0x%08x\n", p->val);
      printf("  new value = 0x%08x\n", new_val);
      p->val = new_val;
      triggered = true;
    }
    p = p->next;
  }

  if (triggered) {
    nemu_state = NEMU_STOP;
  }

  return triggered;
}


