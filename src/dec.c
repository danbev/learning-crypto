#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "dec.h"

void dec_to_binary(int n, int* b, int len) {
  int x = n;
  for (int i = len - 1; i >= 0; i--) {
    int r = x % 2;
    x /= 2;
    b[i] = r;
  }
}

uint8_t dec_to_hex(int dec) {
  if (dec > 9) {
    return dec + 55;
  }
  return dec + 48;
}

