#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "hex.h"
#include "xor.h"
#include "dec.h"
#include "bin.h"
#include "freq.h"

/*
 * Challenge: https://cryptopals.com/sets/1/challenges/3
 *
 * Frequency analysis for English:
 * http://pi.math.cornell.edu/~mec/2003-2004/cryptography/subs/frequencies.html
 */
int main(int argc, char** argv) {
  printf("Cryptopals Set 1, Challenge 3\n");
  uint8_t ciphertext_hex[] = "1B37373331363F78151B7F2B783431333D78397828372D363C78373E783A393B3736";
  printf("Input: ");
  print_hex(ciphertext_hex);
  printf("\n");

  // Convert the ciphertext to binary
  uint8_t* ciphertext_bin = hex_to_binary(ciphertext_hex);
  int bin_len = binary_len_of_hex(ciphertext_hex);
  printf("Input as binary:\n");
  print_binary(ciphertext_bin, bin_len);
  printf("\n");

  uint8_t* key_bin = malloc(bin_len);

  struct score_t score = {3.0, 0.0, NULL};
  for (int i = 8; i <= 255; i++) {
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

    //print_binary(key_bin, bin_len);
    //printf("\n");
    // So we xor the ciphertext and the current key
    uint8_t* plaintext_bin = xor_binary(ciphertext_bin, key_bin, bin_len);

    double s = score_plaintext(plaintext_bin, bin_len);
    if (s < score.s) {
      printf("New score: %f, score.s: %f\n", s, score.s);
      score.s = s;
      score.key = i;
      if (score.binary != NULL) {
        free(score.binary);
      }
      score.binary = plaintext_bin;
      printf("Potential solution: "), print_binary_as_chars(score.binary, bin_len);
      printf("\n");
    } else {
      free(plaintext_bin);
    }
  }
  printf("Solution with lowest score (closest match): %f\n", score.s);
  print_binary_as_chars(score.binary, bin_len);
  free(score.binary);
  free(ciphertext_bin);
  return 0;
}
