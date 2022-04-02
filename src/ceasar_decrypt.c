#include <stdio.h>
#include <stdlib.h>

#include "ceasar.h"

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Usage: ceasar_decrypt <cipertext> <key>\n");
    exit(-1);
  }
  char* ciphertext = argv[1];
  char key = *argv[2];
  printf("ciphertext: %s, key: %c\n", ciphertext, key);
  char* plaintext = ceasar_decrypt(ciphertext, key);
  printf("plaintext: %s\n", plaintext);
  return 0;
}
