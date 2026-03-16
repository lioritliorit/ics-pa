#include "nemu.h"
#include "monitor/expr.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <ctype.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NE, TK_AND, TK_OR, TK_NOT,
  TK_NUM, TK_HEX, TK_REG,
  TK_DEREF, TK_NEG
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {
  {" +", TK_NOTYPE},    // spaces
  {"0x[0-9a-fA-F]+", TK_HEX},
  {"[0-9]+", TK_NUM},
  {"\\$[a-zA-Z][a-zA-Z0-9]*", TK_REG},
  {"==", TK_EQ},
  {"!=", TK_NE},
  {"&&", TK_AND},
  {"\\|\\|", TK_OR},
  {"!", TK_NOT},
  {"\\+", '+'},
  {"\\-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"\\(", '('},
  {"\\)", ')'}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[64];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        if (rules[i].token_type != TK_NOTYPE) {
          if (nr_token >= 64) {
            printf("expression too long\n");
            return false;
          }
          tokens[nr_token].type = rules[i].token_type;
          int len = substr_len;
          if (len >= (int)sizeof(tokens[nr_token].str)) len = sizeof(tokens[nr_token].str) - 1;
          strncpy(tokens[nr_token].str, substr_start, len);
          tokens[nr_token].str[len] = '\0';
          nr_token++;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  /* Handle unary operators: negative sign and dereference. */
  for (i = 0; i < nr_token; i ++) {
    if (tokens[i].type == '-') {
      if (i == 0 || (tokens[i - 1].type != TK_NUM && tokens[i - 1].type != TK_HEX &&
                     tokens[i - 1].type != TK_REG && tokens[i - 1].type != ')')) {
        tokens[i].type = TK_NEG;
      }
    }
    else if (tokens[i].type == '*') {
      if (i == 0 || (tokens[i - 1].type != TK_NUM && tokens[i - 1].type != TK_HEX &&
                     tokens[i - 1].type != TK_REG && tokens[i - 1].type != ')')) {
        tokens[i].type = TK_DEREF;
      }
    }
  }

  return true;
}

static uint32_t reg_str2val(const char *s) {
  int i;
  for (i = 0; i < 8; i ++) {
    if (strcmp(s, regsl[i]) == 0) return reg_l(i);
    if (strcmp(s, regsw[i]) == 0) return reg_w(i);
    if (strcmp(s, regsb[i]) == 0) return reg_b(i);
  }
  return 0;
}

static bool check_parentheses(int p, int q) {
  if (tokens[p].type != '(' || tokens[q].type != ')') return false;

  int cnt = 0;
  for (int i = p; i <= q; i ++) {
    if (tokens[i].type == '(') cnt++;
    else if (tokens[i].type == ')') {
      cnt--;
      if (cnt == 0 && i < q) return false;
    }
  }
  return cnt == 0;
}

static int op_precedence(int type) {
  switch (type) {
    case TK_OR: return 0;
    case TK_AND: return 1;
    case TK_EQ:
    case TK_NE: return 2;
    case '+':
    case '-': return 3;
    case '*':
    case '/': return 4;
    case TK_NEG:
    case TK_DEREF:
    case TK_NOT: return 5;
    default: return 0;
  }
}

static uint32_t eval(int p, int q, bool *success) {
  if (p > q) {
    *success = false;
    return 0;
  }

  if (p == q) {
    Token *t = &tokens[p];
    uint32_t val = 0;
    switch (t->type) {
      case TK_NUM:
        val = strtoul(t->str, NULL, 10);
        break;
      case TK_HEX:
        val = strtoul(t->str, NULL, 16);
        break;
      case TK_REG:
        val = reg_str2val(t->str + 1); /* skip '$' */
        break;
      default:
        *success = false;
        return 0;
    }
    *success = true;
    return val;
  }

  if (check_parentheses(p, q)) {
    return eval(p + 1, q - 1, success);
  }

  int min_precedence = 100;
  int main_op = -1;
  int parentheses = 0;

  for (int i = p; i <= q; i ++) {
    int type = tokens[i].type;
    if (type == '(') {
      parentheses++;
      continue;
    }
    if (type == ')') {
      parentheses--;
      continue;
    }
    if (parentheses != 0) continue;

    if (type == TK_NEG || type == TK_DEREF || type == TK_NOT) {
      int prec = op_precedence(type);
      if (prec <= min_precedence) {
        min_precedence = prec;
        main_op = i;
      }
      continue;
    }

    if (type == TK_EQ || type == TK_NE || type == TK_AND || type == TK_OR ||
        type == '+' || type == '-' || type == '*' || type == '/') {
      int prec = op_precedence(type);
      if (prec <= min_precedence) {
        min_precedence = prec;
        main_op = i;
      }
    }
  }

  if (main_op == -1) {
    *success = false;
    return 0;
  }

  int op = tokens[main_op].type;
  if (op == TK_NEG) {
    uint32_t val = eval(main_op + 1, q, success);
    return *success ? (uint32_t)(- (int32_t)val) : 0;
  }
  if (op == TK_DEREF) {
    uint32_t addr = eval(main_op + 1, q, success);
    return *success ? vaddr_read(addr, 4) : 0;
  }
  if (op == TK_NOT) {
    uint32_t val = eval(main_op + 1, q, success);
    return *success ? (val == 0 ? 1 : 0) : 0;
  }

  bool success_l, success_r;
  uint32_t val1 = eval(p, main_op - 1, &success_l);
  uint32_t val2 = eval(main_op + 1, q, &success_r);
  if (!success_l || !success_r) {
    *success = false;
    return 0;
  }

  uint32_t result = 0;
  switch (op) {
    case '+': result = val1 + val2; break;
    case '-': result = val1 - val2; break;
    case '*': result = val1 * val2; break;
    case '/': result = val2 == 0 ? 0 : val1 / val2; break;
    case TK_EQ: result = (val1 == val2); break;
    case TK_NE: result = (val1 != val2); break;
    case TK_AND: result = (val1 && val2); break;
    case TK_OR: result = (val1 || val2); break;
    default: *success = false; return 0;
  }

  *success = true;
  return result;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  uint32_t val = eval(0, nr_token - 1, success);
  return val;
}
