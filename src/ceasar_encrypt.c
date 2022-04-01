#include <stdio.h>
#include <stdlib.h>

#include "ceasar.h"

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Usage: ceasar_encrypt <string> <key>\n");
    exit(-1);
  }
  char* input = argv[1];
  char key = *argv[2];
  printf("input: %s, key: %c\n", input, key);
  char* ciphertext = ceasar_encrypt(input, key);
  printf("ciphertext: %s\n", ciphertext);
  return 0;
}
