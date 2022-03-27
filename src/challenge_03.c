#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "hex.h"
#include "xor.h"
#include "util.h"

// Generated using ./out/freq gatsby.txt
double sampled_freq[] = {
  0.077432, // a 
  0.014022, // b 
  0.026657, // c 
  0.049208, // d 
  0.134645, // e 
  0.025036, // f 
  0.017007, // g 
  0.057198, // h 
  0.062948, // i 
  0.001268, // j 
  0.005085, // k 
  0.037062, // l 
  0.030277, // m 
  0.071253, // n 
  0.073800, // o 
  0.017513, // p 
  0.000950, // q 
  0.061072, // r 
  0.061263, // s 
  0.087605, // t 
  0.030427, // u 
  0.011137, // v 
  0.021681, // w 
  0.001988, // x 
  0.022836, // y 
  0.000629, // z 
  0.053439, // A 
  0.032315, // B 
  0.026908, // C 
  0.016472, // D 
  0.039105, // E 
  0.027160, // F 
  0.025525, // G 
  0.039482, // H 
  0.400352, // I 
  0.008927, // J 
  0.003898, // K 
  0.016472, // L 
  0.038225, // M 
  0.017478, // N 
  0.020621, // O 
  0.022507, // P 
  0.000126, // Q 
  0.013706, // R 
  0.044009, // S 
  0.082233, // T 
  0.008676, // U 
  0.005533, // V 
  0.036213, // W 
  0.000251, // X 
  0.020370, // Y 
  0.000000, // Z 
};

double frequency_for(char c) {
  if (c >= 65 && c <= 90) {
    return sampled_freq[c - 65];
  } 
  if (c >= 97 && c <= 122) {
    return sampled_freq[(c - 97) + 26];
  } 
  return 0.0;
}

int index_of(char c) {
  if (c >= 65 && c <= 90) {
    return c - 65;
  } 
  if (c >= 97 && c <= 122) {
    return (c - 97) + 26;
  } 
  return -1;
}

const int freq_len = sizeof(sampled_freq) / sizeof(double);

struct score_t {
  double s;
  int key;
  uint8_t* binary;
};

double score_plaintext(uint8_t* bin, int bin_len) {
  double score = 0.0;
  // Calculate the frequencies of the plaintext.
  int frequencies[52] = {0};
  int byte_len = 0;
  for (int i = 0; i < bin_len;) {
    int dec = bin[i++] * 128 + bin[i++] * 64 + bin[i++] * 32 + bin[i++] * 16 +
      bin[i++] * 8 + bin[i++] * 4 + bin[i++] * 2 + bin[i++] * 1;
    int idx = index_of(dec);
    //printf("%c", dec);
    if (idx != -1) {
      frequencies[idx]++;
    }
    byte_len++;
  }

  for (int i = 0; i < 52; i++) {
    double f = (double) frequencies[i] / byte_len;
    score += sampled_freq[i] - f;
  }
  return score;
}

void print_bin_as_chars(uint8_t* bin, int bin_len) {
  int byte_len = 0;
  for (int i = 0; i < bin_len;) {
    int dec = bin[i++] * 128 + bin[i++] * 64 + bin[i++] * 32 + bin[i++] * 16 +
      bin[i++] * 8 + bin[i++] * 4 + bin[i++] * 2 + bin[i++] * 1;
    printf("%c", dec);
  }
  printf("\n");
}

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
      printf("Potential solution: "), print_bin_as_chars(score.binary, bin_len);
      printf("\n");
    } else {
      free(plaintext_bin);
    }
  }
  printf("Solution with lowest score (closest match): %f\n", score.s);
  print_bin_as_chars(score.binary, bin_len);
  free(score.binary);
  free(ciphertext_bin);
  return 0;
}
