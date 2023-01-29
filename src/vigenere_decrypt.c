#include <stdio.h>
#include <stdlib.h>

#include "vigenere.h"
#include "freq.h"

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Usage: vigenere_decrypt <ciphertext> <keyword>\n");
    exit(-1);
  }

  char* ciphertext = argv[1];
  char* keyword = argv[2];
  char* plaintext = vigenere_decrypt(ciphertext, keyword);
  printf("plaintext: %s\n", plaintext);

  double prob_a_and_a = sampled_freq[0] * sampled_freq[0];
  printf("%f\n", prob_a_and_a);

  double dot_product = 0.0;
  for (int i = 0; i < freq_len; i++) {
    dot_product += sampled_freq[i] * sampled_freq[i];
  }
  printf("standard_freq dot_product: %f\n", dot_product);

  double s[] = {
    0.001988, // x 
    0.022836, // y 
    0.000629, // z 
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
  };

  dot_product = 0.0;
  for (int i = 0; i < freq_len; i++) {
    dot_product += s[i] * s[i];
  }
  printf("shifted freq dot_product: %f\n", dot_product);

  double combined_dot_product = 0.0;
  for (int i = 0; i < freq_len; i++) {
    combined_dot_product += sampled_freq[i] * s[i];
  }
  printf("sampled_freq dot shifted_freq: %f\n", combined_dot_product);

  return 0;
}
