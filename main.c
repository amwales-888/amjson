#include <stdio.h>

int main(int argc, char **argv) {

  int i;

  for (i=0; i<256; i++) {

    if ((i >= '0') && (i <= '9')) {
      printf("1, ");
    } else {
      printf("0, ");
    }
  }
}
