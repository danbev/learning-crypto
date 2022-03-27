#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

void print_hex(uint8_t* hex) {
  int len = strlen(hex);
  for (int i = 0; i < len; i++) {
    printf("%c", hex[i]);
  }
  printf("\n");
}

int binary_len_of_hex(uint8_t* hex) {
  int len = strlen(hex);
  return len * 4;
}

uint8_t* hex_to_binary(uint8_t* hex) {
  int len = strlen(hex);
  int b_len = len * 4;
  uint8_t* arr = (uint8_t*) malloc(b_len);

  for (int i = 0, k = b_len, j = 0; i < len; i++) {
    int dec = 0;
    if (hex[i] >= 48 && hex[i] <= 57) {
      dec += hex[i] - 48;
    } else if (hex[i] >= 65 && hex[i] <= 70) {
      dec += hex[i] - 55;
    } else if (hex[i] >= 97 && hex[i] <= 103) {
      dec += hex[i] - 87;
    }

    int idx = i * 4 + 3;
    int q = dec;
    while (q > 0) {
      int r = q % 2;
      q = q / 2;
      arr[idx--] = r;
    }
  }
  return arr;
}
