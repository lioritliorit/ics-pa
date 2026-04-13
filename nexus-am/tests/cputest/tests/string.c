#include "trap.h"

char *s[] = {
	"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 
	"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab",
	"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
	", World!\n",
	"Hello, World!\n",
	"#####"
};

char str1[] = "Hello";
char str[20];
volatile unsigned int dbg_p1, dbg_p2, dbg_c1, dbg_c2, dbg_ret;
volatile unsigned int dbg_idx, dbg_d1, dbg_d2;

int main() {
	nemu_assert(strcmp(s[0], s[2]) == 0);
	nemu_assert(strcmp(s[0], s[1]) == -1);
	nemu_assert(strcmp(s[0] + 1, s[1] + 1) == -1);
	// Debug aid: expose actual strcmp return value via trap code.
	// 0x2xxxx encodes low 16 bits of strcmp result.
	int t = strcmp(s[0] + 2, s[1] + 2);
	if (t != -1) {
		dbg_p1 = (unsigned int)(uintptr_t)(s[0] + 2);
		dbg_p2 = (unsigned int)(uintptr_t)(s[1] + 2);
		dbg_c1 = (unsigned int)(unsigned char)(s[0][2]);
		dbg_c2 = (unsigned int)(unsigned char)(s[1][2]);
		dbg_ret = (unsigned int)t;
		dbg_idx = 0xffffffffu;
		dbg_d1 = 0;
		dbg_d2 = 0;
		for (unsigned int i = 0; i < 64; i++) {
			unsigned char a = (unsigned char)s[0][2 + i];
			unsigned char b = (unsigned char)s[1][2 + i];
			if (a != b) {
				dbg_idx = i;
				dbg_d1 = a;
				dbg_d2 = b;
				break;
			}
			if (a == '\0') break;
		}
		_halt(0x20000 | (t & 0xffff));
	}
	nemu_assert(strcmp(s[0] + 3, s[1] + 3) == -1);

	nemu_assert(strcmp( strcat(strcpy(str, str1), s[3]), s[4]) == 0);

	nemu_assert(memcmp(memset(str, '#', 5), s[5], 5) == 0);

	return 0;
}
