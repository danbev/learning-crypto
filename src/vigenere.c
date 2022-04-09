#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vigenere.h"
#include "crypto_math.h"

char* vigenere_encrypt(char* plaintext, char* key) {
  int len = strlen(plaintext);
  int key_len = strlen(key);
  char* ciphertext = (char*) malloc(len);
  for (int i = 0, idx = 0, j = 0; i < len; i++) {
    if (plaintext[i] >= 97 && plaintext[i] <= 122) {
      ciphertext[idx] = mod((plaintext[i] - 97) + (key[j] - 97), 26) + 97;
      //printf("c: %c, d: %d, k: %c, e:%c\n", plaintext[i], plaintext[i], key[j], ciphertext[i]);
      j++;
      idx++;
      if (j == key_len) {
        j = 0;
      }
    }
  }
  return ciphertext;
}

char* vigenere_decrypt(char* ciphertext, char* key) {
  int len = strlen(ciphertext);
  int key_len = strlen(key);
  char* plaintext = (char*) malloc(len);
  for (int i = 0, j = 0; i < len; i++) {
    plaintext[i] = mod((ciphertext[i] - 97) - (key[j] - 97), 26) + 97; 
    j++;
    if (j == key_len) {
      j = 0;
    }
  }
  return plaintext;
}
