#include <stdio.h>
#include <string.h>
#include <stdint.h>

uint64_t atoui64(char *ptr, int len) {

  int i = len - 20;
  uint64_t value = (ptr[i+19] - '0');

  static void *gotos[] = {
    &&LL0, &&LL1, &&LL2, &&LL3, &&LL4, 
    &&LL5, &&LL6, &&LL7, &&LL8, &&LL9,
    &&LL10, &&LL11, &&LL12, &&LL13, &&LL14, 
    &&LL15, &&LL16, &&LL17, &&LL18, &&LL19
  };

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


int main( int argc, char **argv, char **envp ) {

  uint64_t x = atoui64(argv[1], strlen(argv[1]));

  printf("%lu", x);

  return 0;
}
