/* Testing Code */

#include <limits.h>
#include <math.h>

/* Routines used by floation point test code */

/* Convert from bit level representation to floating point number */
float u2f(unsigned u) {
  union {
    unsigned u;
    float f;
  } a;
  a.u = u;
  return a.f;
}

/* Convert from floating point number to bit-level representation */
unsigned f2u(float f) {
  union {
    unsigned u;
    float f;
  } a;
  a.f = f;
  return a.u;
}

int test_minusOne(void) {
  return -1;
}
int test_bitAnd(int x, int y)
{
  return x&y;
}
int test_negate(int x) {
  return -x;
}
int test_isEqual(int x, int y)
{
  return x == y;
}
int test_isNegative(int x) {
  return x < 0;
}
int test_isIntMin(int x) {
    return x == 0x80000000;
}
int test_addOverflow(int x, int y)
{
    long long lsum = (long long) x + y;
    return lsum != (int) lsum;
}
