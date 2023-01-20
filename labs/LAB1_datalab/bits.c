/* 
 * CS:APP Data Lab 
 * 
 * <Haozhi Fan 01/20/2020>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  int both1 = x & y;
  int both0 = ~x & ~y;
  int result = (~both1) & (~both0);
  return result;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return 0x80000000;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  int isTmin = ~x;
  int is0 = isTmin + 0x7fffffff + 1;
  return !is0;
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  int condition = x & 0xAAAAAAAA;
  int isAll1 = condition + 0x55555555;
  int is0 = ~isAll1;
  return !is0;
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  int negAllBits = ~x;
  return negAllBits + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  //int diffUp = x + (~'9' + 1);
  //int diffDn = x + (~'0' + 1);
  // int diffSum = diffUp + diffDn
  // return !(x >> 6 | (9 + ~diffSum + 1) >> 31 | (diffSum + 9) >> 31);

  int diffSum = (x << 1) + (~105 + 1);
  return !(x >> 6 | (~diffSum + 10) >> 31 | (diffSum + 9) >> 31);
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  // get right-most 1 or all 0
  int cond = !!x;
  // get all 1s or all 0s
  cond = cond << 31;
  cond = cond >> 31;

  int checkY = cond & y;
  int checkZ = ~cond & z;

  return checkY | checkZ;
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int hasDiffSign = !!((x >> 31) ^ (y >> 31));
  int ifDiffSign = (x >> 31) & hasDiffSign;
  // if x and y has different sign, make sure that y-x is 0 or positive
  int notDiffSign = !hasDiffSign & !((y + (~x + 1)) >> 31); 

  // return 1 if either case evaluates to 1
  return ifDiffSign | notDiffSign;
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  // if x is negative, extract the left most 1
  int isNeg = (x >> 31) & 0x00000001;
  // if x is positive, adding Tmax will overflow
  int isPos = ((x + 0x7fffffff) >> 31) & 0x00000001;
  int result = isNeg | isPos;
  return result ^ 0x00000001;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  // https://github.com/nikitos3000/cmu-15213-m14/blob/master/L1-datalab/bits.c
  int y, result, mask16, mask8, mask4, mask2, mask1, bitnum;

  mask1 = 0x00000002;
  mask2 = 0x0000000C;
  mask4 = 0x000000F0;
  mask8 = 0x0000FF00;
  mask16= 0xFFFF0000;

  result = 1;
  y = x ^ (x >> 31);  //cast the number to positive with the same result

  // Check first 16 bits, if they have at least one bit - result > 16
  bitnum = (!!(y & mask16)) << 4;  // 16 OR zero
  result += bitnum;
  y = y >> bitnum;

  bitnum = (!!(y & mask8)) << 3;  // 8 OR zero
  result += bitnum;
  y = y >> bitnum;

  bitnum = (!!(y & mask4)) << 2;  // 4 OR zero
  result += bitnum;
  y = y >> bitnum;

  bitnum = (!!(y & mask2)) << 1;  // 2 OR zero
  result += bitnum;
  y = y >> bitnum;

  bitnum = !!(y & mask1);  // 1 OR zero
  result += bitnum;
  y = y >> bitnum;

  return result + (y & 1);
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  // define special cases
  unsigned SPECIAL = 0x7f800000;   

  // check if argument is special case
  if ((uf & SPECIAL) == SPECIAL) { return uf; }
  // check if argument is denormalized case
  if ((uf << 1) >> 24 == 0) {
    unsigned signbit = uf & 0x80000000;
    return (uf << 1) | signbit;
  }
  // normalized case
  return uf + (1 << 23);
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  // from float to int, the value will be rounded toward zero
  // no need to check denormalized or special case, see below
  unsigned sign = uf & 0x80000000;
  unsigned exp = (uf & 0x7f800000) >> 23;
  unsigned frac = uf & 0x7fffff;
  unsigned BIAS = 127;
  unsigned E = exp - BIAS;
  if (exp < 127) { return 0; }              // smaller than 1
  if (E >= 31) { return 0x80000000u; }      // integer bigger or equal 2^31
  unsigned result = (1 << E) + (frac >> (23 - E));  // frac part need to multiply 2^E
  if (sign) return ~result + 1;
  return result;
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  unsigned sign = x & 0x80000000;
  unsigned BIAS = 127;
  unsigned MAX = (0x7f000000 >> 23) - BIAS;
  unsigned POS_INF = 0x7f800000;
  // when x < 0, exp filed are all 0s
  if (sign) {
    unsigned mag = ~x + 1;        // get -x
    if (mag > 23) { return 0; }
    unsigned result = 1 << 23;    // smallest mag is 1, so result <= 1/2
    while (mag != 1) { 
      result = result >> 1;
      mag -= 1;
    }
    return result;
  }
  // when x >= 0, frac field are all 0s
  if (x > MAX) { return POS_INF; }
  return (x + BIAS) << 23;
}
