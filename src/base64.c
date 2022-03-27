#include <stdlib.h>
#include "base64.h"
#include "util.h"

char* hex_to_base64(unsigned char* hex) {
  static char base64[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
    'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '+', '/'
  };

  uint8_t* bin = hex_to_binary(hex);
  int bin_len = binary_len_of_hex(hex);

  uint8_t joined_binary[24] = {0};
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
