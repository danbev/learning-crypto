#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "base64.h"
#include "bin.h"
#include "hex.h"
#include "dec.h"
#include "str.h"

static const char base64[64] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
  'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
  'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
  'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
  'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', '+', '/'
};

int index_of(char c) {
  if (c >= 65 && c <= 90) {
    return c - 65;
  }
  if (c >= 97 && c <= 122) {
    return c - 97 + 26;
  }
  if (c >= 48 && c <= 57) {
    return c - 48 + 52;
  }

  return 0;
}


char* hex_to_base64(unsigned char* hex) {
  uint8_t* bin = hex_to_binary(hex);
  int bin_len = binary_len_of_hex(hex);

  char* ret = (char*) malloc(bin_len/6);
  int idx = 0;

  int total = 0;
  for (int i = 0; i < bin_len;) {
    int dec = bin[i++] * 32 + bin[i++] * 16 + bin[i++] * 8 + bin[i++] * 4 +
      bin[i++] * 2 + bin[i++];
    //printf("dec: %d, base64: %c\n", dec, base64[dec]);
    ret[idx++] = base64[dec];
    total++;
  }
  return ret;
}

int dec_to_hex2(int d) {
  int r = 0;
  int q = 0;
  while( q > 0)  {
    r = q % 16;
    q = q /16;
  }
}

uint8_t* base64_to_binary(int n, uint8_t* b) {
  uint8_t* bin = b;
  if (bin == NULL) {
    bin = (uint8_t*) malloc(sizeof(n) * 6);
  }

  int x = n;
  for (int i = 6 - 1; i >= 0; i--) {
    int r = x % 2;
    x /= 2;
    bin[i] = r;
  }
  return bin;
}

int binary_len_of_base64(char* b) {
  return (strlen(b) * 6)/4;
}

uint8_t* base64_to_hex(char* b) {
  int base64_len = strlen(b);
  int bin_len = (base64_len * 6);

  char* bin = malloc(bin_len);

  for (int i = 0, j = 0; i < base64_len; i++) {
    int idx = index_of(b[i]);
    uint8_t* dec_bin = base64_to_binary(idx, NULL);
    for (int q = 0; q < 6; q++, j++) {
      bin[j] = dec_bin[q];
    }
    free(dec_bin);
  }

  return bin_to_hex(bin, bin_len);
}
