#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ham.h"
#include "dec.h"
#include "str.h"
#include "bin.h"

int main(int argc, char** argv) {
  printf("Hamming distance example\n");
  uint8_t* m = dec_to_binary(00, NULL);
  uint8_t* n = dec_to_binary(11, NULL);

  int len = sizeof(uint8_t) * 8;
  printf("m: "), print_binary(m, len);
  printf("n: "), print_binary(n, len);
  int z = hamming_distance(m, n, len);
  printf("z: %d\n", z);

  free(m);
  free(n);

  printf("\n");

  printf("Prerequisite for challenge 6\n");
  char* m_str = "this is a test";
  char* n_str = "wokka wokka!!!";
  m = str_to_binary(m_str);
  n = str_to_binary(n_str);
  len = binary_len_of_str(m_str);

  printf("m: "), print_binary(m, len);
  printf("n: "), print_binary(n, len);

  z = hamming_distance(m, n, len);
  printf("z: %d\n", z);
  assert(z == 37);

  return 0;
}
