/* REMEMBER:
 * - This is the only file that you need to edit!
 * - Declare variables only at the beginning of a function (as in C89).
 * - Do not include <stdio.h> header (it confuses dlc)
 * - Check correctness with ./btest
 * - Check operator constraints with ./dlc bits.c
 * - Run ./grade before you commit/push your solution
 */

/*
 * mul7Div16 - multiply by 7/16, rounding toward 0
 *   Should exactly duplicate the C expression (x*7/16),
 *   including overflow behavior.
 *   Examples: mul7Div16(77) = 33
 *             mul7Div16(-22) = -9
 *             mul7Div16(1073741824) = -67108864 (overflow)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 3
 */
int mul7Div16(int x) {
  int num = (x<<2) + (x<<1) + x; // times 7
  int bias = (num>>31) & 15; //bias calc
  return ( (num+bias) >> 4 ); //divide 16
}

/*
 * byteSwap - swaps the n-th byte and the m-th byte (from the right)
 *  Examples: byteSwap(0x12345678, 1, 3) = 0x56341278
 *            byteSwap(0xDEADBEEF, 0, 2) = 0xDEEFBEAD
 *  You can assume that 0 <= n <= 3, 0 <= m <= 3
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 25
 *  Rating: 3
 */
int byteSwap(int x, int n, int m) {
  int byteMask = 0xff; //1111 1111, for copying single bytes
  int n_times_eight = n << 3; //multiply n by 8
  int m_times_eight = m << 3;//multiply m by 8
  //shift a to desired byte range in x, and copy those values for x
  int nByte = (byteMask << n_times_eight)&x;
  int mByte = (byteMask << m_times_eight)&x;

  //combine byte shift for n and m in one overall variable
  //store the unaffected parts of x
  int nmCombined = (byteMask << n_times_eight) | (byteMask << m_times_eight);
  nmCombined = ~nmCombined & x;

  
  //shift bytes to be swapped to left
  nByte = (nByte >> n_times_eight) & byteMask;
  mByte = (mByte >> m_times_eight) & byteMask;
  //shift bytes to be swapped to new position
  nByte = nByte << m_times_eight;
  mByte = mByte << n_times_eight;
  
  return ( mByte | nByte | nmCombined );
}

/*
 * floatAbs - Return the bit-level equivalent of the absolute value
 *   of the floating point argument. Argument and result are passed
 *   as unsigned int's, but they are to be interpreted as the bit-level
 *   representations of single-precision floating point values (32-bit).
 *   When argument is NaN, return it unchanged.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned floatAbs(unsigned uf) {
  unsigned NaN = 0x7F800001;
  unsigned mask = 0x7FFFFFFF;		
  unsigned res = uf & mask;		
  if (res >= NaN){
    return uf;
  }
  else{
    return res;
  }
}

/*
 * floatIsEqual - Compute f == g for floating point arguments f and g.
 *   Both the arguments are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values (32-bit).
 *   If either argument is NaN, return 0.
 *   +0 and -0 are considered equal.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 25
 *   Rating: 4
 */
int floatIsEqual(unsigned uf, unsigned ug) {
  unsigned signmask = 0x7fffffff;
  unsigned f = signmask & uf;
  unsigned g = signmask & ug;
  unsigned fmask = 0x007fffff;
  unsigned expf = !((f >> 23)^0xff); //see if I have 11111111 in the exp value for f returns 1 if true
  unsigned expg = !((g >> 23) ^0xff);//see if I have 11111111 in the exp value for g

  unsigned c1 = ( ((fmask & uf)^0) && expf);// uf is NaN
  unsigned c2 = ( ((fmask & ug)^0) && expg);// ug is NaN

  if (( !((f)^0) ) && ( !((g)^0) )) { //both are 0
    return 1;
  }

  if ( expf && expg){ //both have 1111 1111 in the exp bits
    if ( c1 || c2) { //either is NaN
      return 0;
    }
    return !(uf^ug); //for +/- inf number
  }

  return !(uf^ug); 
}

/*
 * floatHalf - Return bit-level equivalent of expression 0.5*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatHalf(unsigned uf) {
  unsigned sign = uf & 0x80000000; //separate +/- sign
  unsigned frac = uf & 0x007FFFFF; //separate fraction bits
  unsigned exp = uf & 0x7F800000; //separate exponent bits

  if (exp == 0x7F800000) { // exponent bit is all 1s i.e. NaN or Infinity
    return uf; 
  }

  if ( (!exp) || !((exp)^0x00800000) ) { //first 7 digits of expo bit are 0000000 form or input is 0
     frac = (uf & 0x00FFFFFF) >> 1; //maintain adj exponent and frac bits, then divide by 2
     frac = frac + ( ((uf & 3) >> 1) & (uf & 1) ); //round the number
     return frac | sign; //add sign when returning
  }

  return frac | sign | ((exp - 1) & 0x7F800000); 
}
