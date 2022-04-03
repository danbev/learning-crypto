#include <stdio.h>
#include <stdlib.h>

#include "vigenere.h"

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Usage: vigenere_decrypt <ciphertext> <keyword>\n");
    exit(-1);
  }

  char* ciphertext = argv[1];
  char* keyword = argv[2];
  char* plaintext = vigenere_decrypt(ciphertext, keyword);
  printf("plaintext: %s\n", plaintext);

  return 0;
}
