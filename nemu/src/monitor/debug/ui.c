#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_p(char *args);
static int cmd_x(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);

static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Single step execution", cmd_si },
  { "info", "Show registers or watchpoints", cmd_info },
  { "p", "Evaluate expression", cmd_p },
  { "x", "Scan memory: x N EXPR", cmd_x },
  { "w", "Set watchpoint", cmd_w },
  { "d", "Delete watchpoint", cmd_d },
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
  int n = 1;
  if (args) {
    char *arg = strtok(args, " ");
    if (arg) {
      n = strtol(arg, NULL, 10);
      if (n <= 0) { n = 1; }
    }
  }
  cpu_exec(n);
  return 0;
}

static int cmd_info(char *args) {
  if (!args) {
    printf("Usage: info r\n");
    printf("       info w\n");
    return 0;
  }

  char *subcmd = strtok(args, " ");
  if (subcmd == NULL) {
    printf("Usage: info r\n");
    return 0;
  }

  if (strcmp(subcmd, "r") == 0) {
    int i;
    for (i = R_EAX; i <= R_EDI; i ++) {
      printf("%s = 0x%08x\n", regsl[i], reg_l(i));
    }
    printf("eip = 0x%08x\n", cpu.eip);
  }
  else if (strcmp(subcmd, "w") == 0) {
    list_wp();
  }
  else {
    printf("Unknown subcommand '%s'\n", subcmd);
  }

  return 0;
}

static int cmd_p(char *args) {
  if (!args) {
    printf("Usage: p EXPR\n");
    return 0;
  }

  bool success;
  uint32_t val = expr(args, &success);
  if (!success) {
    printf("Bad expression\n");
    return 0;
  }

  printf("0x%08x\n", val);
  return 0;
}

static int cmd_x(char *args) {
  if (!args) {
    printf("Usage: x N EXPR\n");
    return 0;
  }

  char *n_str = strtok(args, " ");
  if (n_str == NULL) {
    printf("Usage: x N EXPR\n");
    return 0;
  }

  int N = strtol(n_str, NULL, 10);
  if (N <= 0) {
    printf("N should be > 0\n");
    return 0;
  }

  char *expr_str = n_str + strlen(n_str);
  while (*expr_str == ' ') expr_str++;
  if (*expr_str == '\0') {
    printf("Usage: x N EXPR\n");
    return 0;
  }

  bool success;
  uint32_t addr = expr(expr_str, &success);
  if (!success) {
    printf("Bad expression\n");
    return 0;
  }

  for (int i = 0; i < N; i ++) {
    uint32_t data = vaddr_read(addr + i * 4, 4);
    printf("0x%08x: 0x%08x\n", addr + i * 4, data);
  }

  return 0;
}

static int cmd_w(char *args) {
  if (!args) {
    printf("Usage: w EXPR\n");
    return 0;
  }

  WP *wp = new_wp(args);
  if (wp) {
    printf("Watchpoint %d: %s\n", wp->NO, wp->expr);
  }

  return 0;
}

static int cmd_d(char *args) {
  if (!args) {
    printf("Usage: d N\n");
    return 0;
  }

  int no = strtol(args, NULL, 10);
  free_wp(no);
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
