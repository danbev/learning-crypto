#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint8_t* str_to_binary(uint8_t* str) {
  int len = strlen(str);
  int b_len = len * 8;
  uint8_t* bin = (uint8_t*) malloc(b_len);
  for (int i = 0; i < len; i++) {
    int idx = (i*8-1) + 8;
    int q = str[i];
    while (q > 0) {
      int r = q % 2;
      q = q / 2;
      bin[idx--] = r;
    }
    if (idx < 7) {
      bin[idx--] = 0;
    }
  }
  return bin;
}

int binary_len_of_str(uint8_t* str) {
  int len = strlen(str);
  return len * 8;
}
