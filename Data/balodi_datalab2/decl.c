#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define TMin INT_MIN
#define TMax INT_MAX

#include "btest.h"
#include "bits.h"

test_rec test_set[] = {
 {"mul7Div16", (funct_t) mul7Div16, (funct_t) test_mul7Div16, 1,
    "! ~ & ^ | + << >>", 10, 3,
  {{-(1<<28)-1, (1<<28)-1},{TMin,TMax},{TMin,TMax}}},
{"byteSwap", (funct_t) byteSwap, (funct_t) test_byteSwap, 3,
     "! ~ & ^ | + << >>", 25, 3,
    {{TMin, TMax},{0,3},{0,3}}},
 {"floatAbs", (funct_t) floatAbs, (funct_t) test_floatAbs, 1,
    "$", 10, 2,
     {{1, 1},{1,1},{1,1}}},
 {"floatIsEqual", (funct_t) floatIsEqual, (funct_t) test_floatIsEqual, 2,
    "$", 25, 4,
     {{1, 1},{1,1},{1,1}}},
 {"floatHalf", (funct_t) floatHalf, (funct_t) test_floatHalf, 1,
    "$", 30, 4,
     {{1, 1},{1,1},{1,1}}},
  {"", NULL, NULL, 0, "", 0, 0,
   {{0, 0},{0,0},{0,0}}}
};
