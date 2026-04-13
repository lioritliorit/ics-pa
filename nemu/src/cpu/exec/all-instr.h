#include "cpu/exec.h"

void decode_Jb(vaddr_t *eip);
void decode_SI(vaddr_t *eip);

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);

make_EHelper(push_r);
make_EHelper(pop_r);
make_EHelper(push);
make_EHelper(pop);
make_EHelper(push_rm);

make_EHelper(add);
make_EHelper(or);
make_EHelper(adc);
make_EHelper(sbb);
make_EHelper(and);
make_EHelper(sub);
make_EHelper(xor);
make_EHelper(cmp);

make_EHelper(inc);
make_EHelper(dec);
make_EHelper(neg);

make_EHelper(test);
make_EHelper(not);
make_EHelper(shl);
make_EHelper(shr);
make_EHelper(sar);
make_EHelper(setcc);

make_EHelper(mul);
make_EHelper(imul1);
make_EHelper(imul2);
make_EHelper(imul3);
make_EHelper(div);
make_EHelper(idiv);

make_EHelper(jmp);
make_EHelper(jcc);
make_EHelper(jmp_rm);
make_EHelper(call);
make_EHelper(ret);
make_EHelper(call_rm);

make_EHelper(leave);
make_EHelper(cltd);
make_EHelper(cwtl);
make_EHelper(movsx);
make_EHelper(movzx);
make_EHelper(lea);

make_EHelper(nop);

make_EHelper(gp1);
make_EHelper(gp2);
make_EHelper(gp3);
make_EHelper(gp4);
make_EHelper(gp5);
make_EHelper(gp7);

make_EHelper(2byte_esc);

make_EHelper(salc);
