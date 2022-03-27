#include <stdint.h>
#include <stdlib.h>
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

uint8_t* str_to_binary(uint8_t* str) {
  int len = strlen(str);
  int b_len = len * 8;
  uint8_t* bin = (uint8_t*) malloc(b_len);
  for (int i = 0; i < len; i++) {
    int idx = i * 8;
    int q = str[i];
    while (q > 0) {
      int r = q % 2;
      q = q / 2;
      bin[idx--] = r;
    }
  }
  return bin;
}

int binary_len_of_str(uint8_t* str) {
  int len = strlen(str);
  return len * 8;
}
