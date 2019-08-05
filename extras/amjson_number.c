/* -------------------------------------------------------------------- *

Copyright 2012 Angelo Masci

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the 
"Software"), to deal in the Software without restriction, including 
without limitation the rights to use, copy, modify, merge, publish, 
distribute, sublicense, and/or sell copies of the Software, and to permit 
persons to whom the Software is furnished to do so, subject to the 
following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR 
THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 * -------------------------------------------------------------------- */

#include "amjson.h"
#include "extras/amjson_number.h"


#define COMPUTEDGOTO
/* #define USESWITCH */
/* #define USELOOP */

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
#ifdef COMPUTEDGOTO
uint64_t amjson_atou64(char *ptr, jsize_t len) {

  if (len <= 20) {

    static void *gotos[] = {
      &&LL0, &&LL1, &&LL2, &&LL3, &&LL4, 
      &&LL5, &&LL6, &&LL7, &&LL8, &&LL9,
      &&LL10, &&LL11, &&LL12, &&LL13, &&LL14, 
      &&LL15, &&LL16, &&LL17, &&LL18, &&LL19
    };

    int i = len - 20;
    uint64_t value = (ptr[i+19] - '0');

    goto *gotos[len-1];

  LL19:
    value += 10000000000000000000UL * (ptr[i+0] - '0');
  LL18:
    value += 1000000000000000000UL * (ptr[i+1] - '0');
  LL17:
    value += 100000000000000000UL * (ptr[i+2] - '0');
  LL16:
    value += 10000000000000000UL * (ptr[i+3] - '0');
  LL15:
    value += 1000000000000000UL * (ptr[i+4] - '0');
  LL14:
    value += 100000000000000UL * (ptr[i+5] - '0');
  LL13:
    value += 10000000000000UL * (ptr[i+6] - '0');
  LL12:
    value += 1000000000000UL * (ptr[i+7] - '0'); 
  LL11:
    value += 100000000000UL * (ptr[i+8] - '0');
  LL10:
    value += 10000000000UL * (ptr[i+9] - '0');
  LL9:
    value += 1000000000UL * (ptr[i+10] - '0');
  LL8:
    value += 100000000UL * (ptr[i+11] - '0');
  LL7:
    value += 10000000UL * (ptr[i+12] - '0');
  LL6:
    value += 1000000UL * (ptr[i+13] - '0');
  LL5:
    value += 100000UL * (ptr[i+14] - '0'); 
  LL4:
    value += 10000UL * (ptr[i+15] - '0');
  LL3:
    value += 1000UL * (ptr[i+16] - '0');
  LL2:
    value += 100UL * (ptr[i+17] - '0');
  LL1:
    value += 10UL * (ptr[i+18] - '0');
  LL0:
    return value;
  }

  return 0;
}
#endif

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
#ifdef USESWITCH
uint64_t amjson_atou64(char *ptr, jsize_t len) {

  if (len <= 20) {

    uint64_t value = 0;

    switch (len-1) {
    case 19:
      value += 10000000000000000000UL * (unsigned char)(*ptr++ - '0');
    case 18:
      value += 1000000000000000000UL * (unsigned char)(*ptr++ - '0');
    case 17:
      value += 100000000000000000UL * (unsigned char)(*ptr++ - '0');
    case 16:
      value += 10000000000000000UL * (unsigned char)(*ptr++ - '0');
    case 15:
      value += 1000000000000000UL * (unsigned char)(*ptr++ - '0');
    case 14:
      value += 100000000000000UL * (unsigned char)(*ptr++ - '0');
    case 13:
      value += 10000000000000UL * (unsigned char)(*ptr++ - '0');
    case 12:
      value += 1000000000000UL * (unsigned char)(*ptr++ - '0');
    case 11:
      value += 100000000000UL * (unsigned char)(*ptr++ - '0');
    case 10:
      value += 10000000000UL * (unsigned char)(*ptr++ - '0');
    case 9:
      value += 1000000000UL * (unsigned char)(*ptr++ - '0');
    case 8:
      value += 100000000UL * (unsigned char)(*ptr++ - '0');
    case 7:
      value += 10000000UL * (unsigned char)(*ptr++ - '0');
    case 6:
      value += 1000000UL * (unsigned char)(*ptr++ - '0');
    case 5:
      value += 100000UL * (unsigned char)(*ptr++ - '0');
    case 4:
      value += 10000UL * (unsigned char)(*ptr++ - '0');
    case 3:
      value += 1000UL * (unsigned char)(*ptr++ - '0');
    case 2:
      value += 100UL * (unsigned char)(*ptr++ - '0');
    case 1:
      value += 10UL * (unsigned char)(*ptr++ - '0');
    case 0:
      return value + (unsigned char)(*ptr++ - '0');
    }
  }

  return 0;
}
#endif

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
#ifdef USELOOP
uint64_t amjson_atou64(char *ptr, jsize_t len) {

  uint64_t value = (unsigned char)(*ptr++ - '0');
  
  while (--len) {
    
    value *= 10;
    value += (unsigned char)(*ptr++ - '0');
  }
  
  return value;
}
#endif

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
