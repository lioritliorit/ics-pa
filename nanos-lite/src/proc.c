#include "proc.h"

#define MAX_NR_PROC 4

PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;
int current_game = 0;  // 0 = pal, 2 = videotest

void switch_game(void) {
  current_game = (current_game == 0 ? 2 : 0);
  Log("Switched to game %d", current_game);
}

uintptr_t loader(_Protect *as, const char *filename);

void load_prog(const char *filename) {
  int i = nr_proc ++;
  _protect(&pcb[i].as);

  uintptr_t entry = loader(&pcb[i].as, filename);

  pcb[i].cur_brk = 0;
  pcb[i].max_brk = 0;

  // TODO: remove the following three lines after you have implemented _umake()
  // _switch(&pcb[i].as);
  // current = &pcb[i];
  // ((void (*)(void))entry)();

  _Area stack;
  stack.start = pcb[i].stack;
  stack.end = stack.start + sizeof(pcb[i].stack);

  pcb[i].tf = _umake(&pcb[i].as, stack, stack, (void *)entry, NULL, NULL);
}

_RegSet* schedule(_RegSet *prev) {
  
  // 保存当前进程的上下文
  if (current != NULL && prev != NULL) {
    current->tf = prev;
  }
  
  // 第一次时，current 可能是 NULL，设置为 &pcb[1] 来确保第一次运行游戏
  if (current == NULL) {
    current = &pcb[1];
  }
  
  // 在当前游戏和 hello 程序之间切换
  if (current == &pcb[1]) {
    // hello -> 游戏
    current = &pcb[current_game];
  } else {
    // 游戏 -> hello
    current = &pcb[1];
  }
  
  // 切换到新地址空间
  _switch(&current->as);
  
  // 返回新上下文
  return current->tf;
}
