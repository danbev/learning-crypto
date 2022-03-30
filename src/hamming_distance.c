#include <stdio.h>

#include "ham.h"
#include "dec.h"
#include "bin.h"

int main(int argc, char** argv) {
  printf("Hamming distance example\n");
  uint8_t* m = dec_to_binary(10, NULL);
  uint8_t* n = dec_to_binary(11, NULL);

  int len = sizeof(uint8_t) * 8;
  printf("m: "), print_binary(m, len);
  printf("n: "), print_binary(n, len);
  int z = hamming_distance(m, n, len);
  printf("z: %d\n", z);

  return 0;
}
