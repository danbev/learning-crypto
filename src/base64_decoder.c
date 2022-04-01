#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base64.h"
#include "hex.h"

/*
 * Example usage:
 * $ ./out/base64_decoder SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t
 */
int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: base64_gen <string_to_be_encoded>\n");
    return -1;
  }
  char* str = argv[1];
  printf("Input base64: %s\n", str);
  uint8_t* hex = base64_to_hex(str);
  print_hex(hex, binary_len_of_base64(str));

  free(hex);

  return 0;
}
