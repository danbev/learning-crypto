#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hex.h"

void print_binary(uint8_t* b, int size) {
  for (int i = 0; i < size; i++) {
    printf("%d", b[i]);
  }
  printf("\n");
}

int bin_to_dec(uint8_t* b, int len) {
  int dec = 0;
  for (int i = 0, j = len-1; i < len; i++, j--) { 
    dec += (int) pow(2, j) * b[i];
  } 
  return dec;
}

uint8_t* bin_to_hex(uint8_t* bin, int len) {
  int h_len = len/4;
  uint8_t* hex = (uint8_t*) malloc(h_len);

  uint8_t nibble[4];
  int idx = 0;
  for (int i = 0, j=0, idx = 0; j < h_len; idx+=4, j++ ) {
    nibble[0] = bin[i++];
    nibble[1] = bin[i++];
    nibble[2] = bin[i++];
    nibble[3] = bin[i++];
    int d = bin_to_dec(nibble, 4);

    int r = d % 16;
    d = d / 16;
    if (r > 9) {
      hex[j] += r + 55;
    } else {
      hex[j] = r + 48;
    }
    //printf("j: %d, %d %d %d %d = %c\n", j, nibble[0], nibble[1], nibble[2], nibble[3], hex[j]);
  }
  return hex;
}

void dec_to_binary(int n, int* b, int len) {
  int x = n;
  for (int i = len - 1; i >= 0; i--) {
    int r = x % 2;
    x /= 2;
    b[i] = r;
  }
}
