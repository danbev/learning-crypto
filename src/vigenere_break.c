#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vigenere.h"
#include "freq.h"

int main(int argc, char** argv) {
  char* ciphertext = "tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpyh";
  int len = strlen(ciphertext);
  printf("ciphertext: %s\n", ciphertext);
  printf("len: %d\n", len);

  // Count the occurances of letters in the ciphertext.
  int shifted_count[26] = {0};
  for (int i = 0; i < len; i++) {
    int nr = ciphertext[i] - 97;
    //printf("nr: %d, %c\n", nr, ciphertext[i]);
    shifted_count[nr]++;
  }
  double shifted_freq[26] = {0.0};
  for (int i = 0; i < 26; i++) {
    shifted_freq[i] = shifted_count[i] * sampled_freq[i];
    printf("%c: %d, %f\n", i + 97, shifted_count[i], shifted_freq[i]);
  }

  double dot_product = 0.0;
  for (int i = 0; i < freq_len; i++) {
    dot_product += sampled_freq[i] * sampled_freq[i];
  }
  printf("standard_freq dot_product: %f\n", dot_product);

  dot_product = 0.0;
  for (int i = 0; i < freq_len; i++) {
    dot_product += shifted_freq[i] * shifted_freq[i];
  }
  printf("shifted_freq dot_product: %f\n", dot_product);

  double combined_dot_product = 0.0;
  for (int i = 0; i < freq_len; i++) {
    combined_dot_product += sampled_freq[i] * shifted_freq[i];
  }
  printf("sampled_freq dot shifted_freq: %f\n", combined_dot_product);

  return 0;
}
