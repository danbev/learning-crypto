#include <stdint.h>
#include <math.h>

#include "crypto_math.h"

int hamming_distance(uint8_t* m, uint8_t* n, int len) {
  int z = 0;
  for (int i = 0; i < len; i++) {
    z += mod(m[i] + n[i], 2);
  }
  return z;
}
