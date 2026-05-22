#include "proc.h"

#define MAX_NR_PROC 4

PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;

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
  
  // 第一次时，current 可能是 NULL，设置为 &pcb[1] 来确保第一次运行 pcb[0]
  if (current == NULL) {
    current = &pcb[1];
  }
  
  // 优先级调度：让 pcb[0]（仙剑奇侠传）运行 10 次，才让 pcb[1]（hello）运行 1 次
  static int pal_count = 0;
  if (current == &pcb[0]) {
    pal_count++;
    if (pal_count < 10) {
      // 继续运行仙剑奇侠传，不切换
      _switch(&current->as);
      return current->tf;
    } else {
      // 计数满了，切换到 hello，并重置计数
      pal_count = 0;
      current = &pcb[1];
    }
  } else {
    // hello 只运行 1 次就切回仙剑
    current = &pcb[0];
  }
  
  // 切换到新地址空间
  _switch(&current->as);
  
  // 返回新上下文
  return current->tf;
}
