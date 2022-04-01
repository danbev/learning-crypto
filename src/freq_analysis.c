#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "hex.h"
#include "xor.h"
#include "dec.h"
#include "bin.h"
#include "str.h"
#include "freq.h"

int main(int argc, char** argv) {
  printf("Frequency Analysis exploration\n");
  char* input = "This is my very secret message";
  char key = 'k';
  uint8_t* input_bin = str_to_binary(input);
  int bin_str_len = binary_len_of_str(input);
  printf("Input: %s\n", input);
  printf("Binary:\n"), print_binary(input_bin, bin_str_len);
  print_binary_as_chars(input_bin, bin_str_len);
  printf("Key: %c\n", key);

  uint8_t* binary_key = gen_single_key(key, bin_str_len);
  printf("Binary Key:\n"), print_binary(binary_key, bin_str_len);

  uint8_t* xored = xor_binary(input_bin, binary_key, bin_str_len);
  printf("Xored:\n"), print_binary(xored, bin_str_len);

  uint8_t* ciphertext_hex = bin_to_hex(xored, bin_str_len);
  printf("Xored hex:\n"), print_hex(ciphertext_hex, strlen(ciphertext_hex));

  printf("Input:\n");
  print_hex(ciphertext_hex, strlen(ciphertext_hex));
  printf("\n");

  // Convert the ciphertext to binary
  uint8_t* ciphertext_bin = hex_to_binary(ciphertext_hex);
  int bin_len = binary_len_of_hex(ciphertext_hex);

  uint8_t* key_bin = malloc(bin_len);

  struct score_t score = {3.0, 0.0, NULL};
  for (int i = 0; i <= 255; i++) {
    uint8_t b[8] = {0};
    dec_to_binary(i, b);
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
    if (s < score.s) {
      printf("New score: %f, score.s: %f, key: %d (%c)\n", s, score.s, i, i);
      score.s = s;
      score.key = i;
      if (score.binary != NULL) {
        free(score.binary);
      }
      score.binary = plaintext_bin;
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
