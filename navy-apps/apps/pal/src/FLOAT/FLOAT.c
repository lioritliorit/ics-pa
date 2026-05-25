#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>
#define FLOAT_FACTOR (1 << 16) // 2^16
typedef union
{
  float value;
  struct
  {
    unsigned mantissa : 23;// 尾数部分
    unsigned exponent : 8;// 指数部分
    unsigned sign : 1;// 符号位
  };
} float_raw;

typedef union
{
  FLOAT value;
  struct
  {
    unsigned frac : 16;    // 小数部分
    unsigned integer : 15; // 整数部分
    unsigned sign : 1;     // 符号位
  };
} Float;

FLOAT F_mul_F(FLOAT a_raw, FLOAT b_raw)
{
  // assert(0);
  // return 0;
  Float a = {a_raw}, b = {b_raw};

  int result_sign = a.sign ^ b.sign;

  // 去符号，统一为正数进行乘法
  a.value = a.sign ? -a.value : a.value;
  b.value = b.sign ? -b.value : b.value;

  int int_part = a.integer * b.integer * FLOAT_FACTOR;
  int cross1 = a.frac * b.integer;
  int cross2 = a.integer * b.frac;
  int frac_mul = a.frac * b.frac;
  FLOAT frac_part = (frac_mul / FLOAT_FACTOR) + (frac_mul % FLOAT_FACTOR >= FLOAT_FACTOR / 2);

  FLOAT result = int_part + cross1 + cross2 + frac_part;
  return result_sign ? -result : result;
}

FLOAT F_div_F(FLOAT a, FLOAT b)
{
  // assert(0);
  // return 0;
  assert(b != 0);

  FLOAT dividend = Fabs(a);
  FLOAT divisor = Fabs(b);
  FLOAT quotient = dividend / divisor;
  dividend %= divisor;

  for (int i = 0; i < 16; i++)
  {
    dividend <<= 1;
    quotient <<= 1;
    if (dividend >= divisor)
    {
      dividend -= divisor;
      quotient++;
    }
  }

  // 若符号不同，则结果为负
  if ((a ^ b) & 0x80000000)
  {
    quotient = -quotient;
  }

  return quotient;
}

FLOAT f2F(float a)
{
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */

  // assert(0);
  // return 0;
  float_raw bits = {a};

  if (bits.exponent == 0)
    return 0; // 零或非正规数，FLOAT 精度有限，直接视作 0 处理
  if (bits.exponent == 0xff)
    return bits.sign ? 0x80000000 : 0x7fffffff; // 在定点数 FLOAT 中，我们没有 NaN 或 inf 表示，所以我们直接返回最大/最小的 32 位整数。

  int shift = bits.exponent - 127 + 16;
  if (shift < 0) //小于1，小于最小非零值
    return 0;

  int base = bits.mantissa | (1 << 23); // 补上IEEE 754隐含的1
  shift -= 23;

  int result = (shift < 0) ? (base >> -shift) : (base << shift); // 左移还是右移
  return bits.sign ? -result : result;
}

FLOAT Fabs(FLOAT a)
{
  // assert(0);
  // return 0;
  return a < 0 ? -a : a;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x)
{
  FLOAT dt, t = int2F(2);

  do
  {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while (Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y)
{
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do
  {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while (Fabs(dt) > f2F(1e-4));

  return t;
}
