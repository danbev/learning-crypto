#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "hex.h"
#include "xor.h"
#include "dec.h"
#include "bin.h"
#include "freq.h"

void find_key(char* line, struct score_t* score);

/*
 * Challenge: https://cryptopals.com/sets/1/challenges/4
 */
int main(int argc, char** argv) {
  printf("Cryptopals Set 1, Challenge 4\n");

  char* filename = "src/4.txt";
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    printf("Could not locate file: %s\n", filename);
    return -1;
  }
  char* line = NULL;
  size_t len = 0;
  int read = 0;
  struct score_t score = {3.0, 0, NULL};
  while (((read = getline(&line, &len, file)) != -1)) {
    if (line[read-1] == '\n') {
      line[read-1] = '\0';
    }
    find_key(line, &score);
  }
  int bin_len = binary_len_of_hex(line);
  printf("ciphertext_hex: %s\n", line);
  printf("score.key: %d (%c), score: %f\n", score.key, score.key, score.s);
  print_binary_as_chars(score.binary, bin_len);

  free(score.binary);
  free(line);
  fclose(file);

  return 0;
}

void find_key(char* ciphertext_hex, struct score_t* score) {
  uint8_t* ciphertext_bin = hex_to_binary(ciphertext_hex);
  int bin_len = binary_len_of_hex(ciphertext_hex);

  uint8_t* key_bin = malloc(bin_len);

  for (int i = 0; i <= 255; i++) {
    int b[8] = {0};
    dec_to_binary(i, b, 8);
    for (int y = 0; y < bin_len; y+=8) {
      int idx = y;
      key_bin[idx] = b[0];
      key_bin[idx+1] = b[1];
      key_bin[idx+2] = b[2];
      key_bin[idx+3] = b[3];
      key_bin[idx+4] = b[4];
      key_bin[idx+5] = b[5];
      key_bin[idx+6] = b[6];
      key_bin[idx+7] = b[7];
    }

    uint8_t* plaintext_bin = xor_binary(ciphertext_bin, key_bin, bin_len);

    double s = score_plaintext(plaintext_bin, bin_len);
    if (s < score->s) {
      score->s = s;
      score->key = i;
      if (score->binary != NULL) {
       free(score->binary);
      }
      score->binary = plaintext_bin;
    }
  }
}
