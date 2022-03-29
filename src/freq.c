#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "freq.h"
#include "dec.h"

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
};

const int freq_len = sizeof(sampled_freq) / sizeof(double);

uint8_t* gen_single_key(char key, int len) {
  uint8_t* key_bin = malloc(len);
  int b[8] = {0};
  dec_to_binary(key, b, 8);
  for (int i = 0; i < len; i+=8) {
    int idx = i;
    key_bin[idx] = b[0];
    key_bin[idx+1] = b[1];
    key_bin[idx+2] = b[2];
    key_bin[idx+3] = b[3];
    key_bin[idx+4] = b[4];
    key_bin[idx+5] = b[5];
    key_bin[idx+6] = b[6];
    key_bin[idx+7] = b[7];
  }
  return key_bin;
}

double frequency_for(char c) {
  if (c >= 97 && c <= 122) {
    return sampled_freq[(c - 97)];
  } 
  return 0.0;
}

int index_of(char c) {
  if (c >= 97 && c <= 122) {
    return (c - 97);
  } 
  return -1;
}

double score_plaintext(uint8_t* bin, int bin_len) {
  double score = 0.0;
  // Calculate the frequencies of the plaintext.
  int frequencies[26] = {0};
  int bytes_len = bin_len / 8;
  for (int i = 0; i < bin_len;) {
    int dec = bin[i++] * 128 + bin[i++] * 64 + bin[i++] * 32 + bin[i++] * 16 +
      bin[i++] * 8 + bin[i++] * 4 + bin[i++] * 2 + bin[i++] * 1;
    int idx = index_of(dec);
    if (idx != -1) {
      frequencies[idx]++;
    }
  }

  for (int i = 0; i < 26; i++) {
    double f = (double) frequencies[i] / bytes_len;
    score += sampled_freq[i] - f;
  }
  return score;
}

void print_binary_as_chars(uint8_t* bin, int bin_len) {
  for (int i = 0; i < bin_len;) {
    int dec = bin[i++] * 128 + bin[i++] * 64 + bin[i++] * 32 + bin[i++] * 16 +
      bin[i++] * 8 + bin[i++] * 4 + bin[i++] * 2 + bin[i++] * 1;
    printf("%c", dec);
  }
  printf("\n");
}
