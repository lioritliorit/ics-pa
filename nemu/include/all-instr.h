// This file is auto-generated. Do not edit manually.

#ifndef __ALL_INSTR_H__
#define __ALL_INSTR_H__

#include "cpu/exec.h"

// Function declarations for all instructions
void exec_lidt(vaddr_t *eip);
void exec_mov_r2cr(vaddr_t *eip);
void exec_mov_cr2r(vaddr_t *eip);
void exec_int(vaddr_t *eip);
void exec_iret(vaddr_t *eip);
void exec_in(vaddr_t *eip);
void exec_out(vaddr_t *eip);

// Special instructions
void exec_nop(vaddr_t *eip);
void exec_inv(vaddr_t *eip);
void exec_nemu_trap(vaddr_t *eip);

// Arithmetic instructions
void exec_add(vaddr_t *eip);
void exec_adc(vaddr_t *eip);
void exec_inc(vaddr_t *eip);
void exec_sub(vaddr_t *eip);
void exec_sbb(vaddr_t *eip);
void exec_dec(vaddr_t *eip);
void exec_neg(vaddr_t *eip);
void exec_cmp(vaddr_t *eip);
void exec_mul(vaddr_t *eip);
void exec_imul1(vaddr_t *eip);
void exec_imul2(vaddr_t *eip);
void exec_div(vaddr_t *eip);
void exec_idiv(vaddr_t *eip);

// Logic instructions
void exec_test(vaddr_t *eip);
void exec_and(vaddr_t *eip);
void exec_or(vaddr_t *eip);
void exec_xor(vaddr_t *eip);
void exec_sar(vaddr_t *eip);
void exec_shl(vaddr_t *eip);
void exec_shr(vaddr_t *eip);
void exec_setcc(vaddr_t *eip);
void exec_not(vaddr_t *eip);

// Data movement instructions
void exec_mov(vaddr_t *eip);
void exec_movsx(vaddr_t *eip);
void exec_movzx(vaddr_t *eip);
void exec_lea(vaddr_t *eip);
void exec_push(vaddr_t *eip);
void exec_pop(vaddr_t *eip);
void exec_push_r(vaddr_t *eip);
void exec_pop_r(vaddr_t *eip);
void exec_push_rm(vaddr_t *eip);

// Control flow instructions
void exec_call(vaddr_t *eip);
void exec_call_rm(vaddr_t *eip);
void exec_ret(vaddr_t *eip);
void exec_jmp(vaddr_t *eip);
void exec_jmp_rm(vaddr_t *eip);
void exec_jcc(vaddr_t *eip);
void exec_leave(vaddr_t *eip);

// Group instructions
void exec_gp1(vaddr_t *eip);
void exec_gp2(vaddr_t *eip);
void exec_gp3(vaddr_t *eip);
void exec_gp4(vaddr_t *eip);
void exec_gp5(vaddr_t *eip);
void exec_gp7(vaddr_t *eip);

// 2-byte escape
void exec_2byte_esc(vaddr_t *eip);

// Prefix
void exec_operand_size(vaddr_t *eip);
void exec_cltd(vaddr_t *eip);

#endif