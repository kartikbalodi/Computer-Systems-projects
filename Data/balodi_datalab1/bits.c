/* REMEMBER:
 * - This is the only file that you need to edit!
 * - Declare variables only at the beginning of a function (as in C89).
 * - Do not include <stdio.h> header (it confuses dlc)
 * - Check correctness with ./btest
 * - Check operator constraints with ./dlc bits.c
 * - Run ./grade before you commit/push your solution
 */

/*
Sought assistance during Friday OH noon-1pm
CPs/TAs

*/

/*
 * minusOne - return a value of -1
 * (tip: how is -1 encoded in two's complement?)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 1
 *   Rating: 1
 */

int minusOne(void) {
  return ~0x0;
}

/*
 * bitAnd - return x&y using only ~ and |
 * (tip: always remember De Morgan's laws!)
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 4
 *   Rating: 1
 */
int bitAnd(int x, int y) {

  return ~((~x)|(~y));
}

/*
 * negate - return -x
 * (tip: remember the definition of two's complement!)
 *   Example: negate(1) = -1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 2
 *   Rating: 1
 */
int negate(int x) {
  return ~x+1;
}

/*
 * isEqual - return 1 if x == y, and 0 otherwise
 * (tip: which bitwise op computes the difference?)
 *   Examples: isEqual(5,5) = 1, isEqual(4,5) = 0
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 2
 *   Rating: 2
 */
int isEqual(int x, int y) {
  return !(x^y);
}

/*
 * isNegative - return 1 if x < 0, and 0 otherwise
 * (tip: Which bit distinguishes negative integers?)
 *   Example: isNegative(-1) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 2
 *   Rating: 3
 */
int isNegative(int x) {
  return (x>>31)&1;
}

/*
 * isIntMin - return 1 if x is the minimum signed int, and 0 otherwise
 * (tip: what happens if you subtract a large negative number from INT_MIN?)
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 5
 *   Rating: 3
 */
int isIntMin(int x) {
  // !(min num) && !0
  return !( (x+x) | (!x) );
}

/*
 * addOverflow - return 1 if x+y causes signed overflow, and 0 otherwise
 * (tip: remember the signed overflow rule)
 *   Example: addOverflow(0x80000000,0x80000000) = 1
 *            addOverflow(0x80000000,0x70000000) = 0
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int addOverflow(int x, int y) {

  int c1 = (x>>31)&1;
  int c2 = (y>>31)&1;
  int c3 = !(c1^c2);
  int c4 = x+y;
  int c5 = (c4>>31)&1;

  return c3&(c1^c5);

/*
  int c1 = (x>>30)&3;
  int c2 = (y>>30)&3;

  int c3 = c1&c2;
  int c4 = c3<<31;

  return ( ( !(c1^c2))&( (c4>>31)&1) );
*/
}
