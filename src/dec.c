#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "dec.h"

uint8_t* dec_to_binary(int n, uint8_t* b) {
  uint8_t* bin = b;
  if (bin == NULL) {
    bin = (uint8_t*) malloc(sizeof(n) * 8);
  }

  int x = n;
  for (int i = 8 - 1; i >= 0; i--) {
    int r = x % 2;
    x /= 2;
    bin[i] = r;
  }
  return bin;
}

uint8_t dec_to_hex(int dec) {
  if (dec > 9) {
    return dec + 55;
  }
  return dec + 48;
}
