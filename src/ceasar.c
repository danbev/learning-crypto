#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "ceasar.h"

int mod(int a, int n) {
  int r = a - n * floor(a / n);
  return (r < 0) ? r + n : r;
}

char* ceasar_encrypt(char* plaintext, char key) {
  int len = strlen(plaintext);
  char* ciphertext = malloc(len);
  int nr = key - 97; // assuming lowercase key
  for (int i = 0; i < len; i++) {
    int letter = plaintext[i] - 97;
    int new_value = mod(letter+nr, 26) + 97;
    //printf("i: %d, letter: %d:%c, ciphertext[i] = %c\n", i, letter, plaintext[i], new_value);
    ciphertext[i] = new_value;
  }
  return ciphertext;
}

char* ceasar_decrypt(char* ciphertext, char key) {
  int len = strlen(ciphertext);
  char* plaintext = malloc(len);
  int nr = key - 97;
  for (int i = 0; i < len; i++) {
    int letter = ciphertext[i] - 97;
    int new_value = mod(letter - nr, 26) + 97;
    plaintext[i] = new_value;
  }

  return plaintext;
}
