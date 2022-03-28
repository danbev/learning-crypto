#ifndef SRC_FREQ_H
#define SRC_FREQ_H

#include <stdint.h>
#include <stdlib.h>

extern double sampled_freq[52];
extern const int freq_len;

struct score_t {
  double s;
  int key;
  uint8_t* binary;
};

uint8_t* gen_single_key(char key, int len);
double frequency_for(char c);
int index_of(char c);

double score_plaintext(uint8_t* bin, int bin_len);

void print_binary_as_chars(uint8_t* bin, int bin_len);

#endif /* SRC_FREQ_H */
