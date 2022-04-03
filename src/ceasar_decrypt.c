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
  printf("ciphertext: %s\n", ciphertext);
  char* plaintext = ceasar_decrypt(ciphertext, key);
  printf("plaintext: %s\n", plaintext);

  printf("decrypt without using key:\n");
  char k = find_key(ciphertext);
  printf("key found: %c\n", k);
  printf("using key %c: %s\n", k, ceasar_decrypt(ciphertext, k));
  return 0;
}
