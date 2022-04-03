#include <stdio.h>
#include <stdlib.h>

#include "vigenere.h"

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Usage: vigenere_encrypt <plaintext> <keyword>\n");
    exit(-1);
  }

  char* plaintext = argv[1];
  char* keyword = argv[2];
  char* ciphertext = vigenere_encrypt(plaintext, keyword);
  printf("ciphertext: %s\n", ciphertext);

  return 0;
}
