#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define TMin INT_MIN
#define TMax INT_MAX

#include "btest.h"
#include "bits.h"

test_rec test_set[] = {
 {"minusOne", (funct_t) minusOne, (funct_t) test_minusOne, 0,
    "! ~ & ^ | + << >>", 1, 1,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},
 {"bitAnd", (funct_t) bitAnd, (funct_t) test_bitAnd, 2, "| ~", 4, 1,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},
 {"negate", (funct_t) negate, (funct_t) test_negate, 1,
    "! ~ & ^ | + << >>", 2, 1,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},
 {"isEqual", (funct_t) isEqual, (funct_t) test_isEqual, 2,
    "! ~ & ^ | + << >>", 2, 2,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},
 {"isNegative", (funct_t) isNegative, (funct_t) test_isNegative, 1,
    "! ~ & ^ | + << >>", 2, 3,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},
 {"isIntMin", (funct_t) isIntMin, (funct_t) test_isIntMin, 1, "! ~ & ^ | +", 5, 3,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},
 {"addOverflow", (funct_t) addOverflow, (funct_t) test_addOverflow, 2,
    "! ~ & ^ | + << >>", 15, 3,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},
  {"", NULL, NULL, 0, "", 0, 0,
   {{0, 0},{0,0},{0,0}}}
};
