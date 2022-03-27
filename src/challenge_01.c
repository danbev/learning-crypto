#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "util.h"

int main(int argc, char** argv) {
  printf("Cryptopals Set 1, Challenge 1 hex to base64 conversion\n");
  char* input = "49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d";
  char* expected = "SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t";
  printf("Input:     "), print_hex(input);

  char* base64 = hex_to_base64(input);
  printf("Expected : %s\n", expected);
  printf("Actual   : %s\n", base64);
  assert(strcmp(expected, base64) == 0);

  free(base64);
}
