#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "ceasar.h"
#include "freq.h"

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

char find_key(char* ciphertext) {
  int len = strlen(ciphertext);
  double expected_freq[26] = {0.0};

  // Calculate the number of expected frequency counts for all letters.
  for (int i = 0; i < 26; i++) {
    expected_freq[i] = sampled_freq[i] * len;
  }

  struct key_sum {
    double sum;
    char key;
  };

  struct key_sum highest = {10000.0, -1};

  // Loop through all the possible keys (0-25).
  for (int i = 0; i < 26; i++) {
    int key = i + 97;
    char* decrypted = ceasar_decrypt(ciphertext, key);
    //printf("key: %c, %s\n", key, decrypted);

    // Count the occurences of letters in the decrypted text for the current
    // key.
    int count[26] = {0};
    for (int j = 0; j < len; j++) {
      count[decrypted[j] - 97]++;
    }

    // Calculate chi-squared for each key.
    double sum = 0.0;
    for (int j = 0; j < 26; j++) {
      sum += pow(count[j] - expected_freq[j], 2) / expected_freq[j];
    }
    if (sum < highest.sum) {
      highest.sum = sum;
      highest.key = key;
    }
    //printf("sum: %f, key: %c\n", sum, key);
    free(decrypted);
  }

  return highest.key;
}
