#include <stdint.h>
#include <math.h>

int mod(int a, int n) {
  int r = a - n * floor(a / n);
  return (r < 0) ? r + n : r;
}

int hamming_distance(uint8_t* m, uint8_t* n, int len) {
  int z = 0;
  for (int i = 0; i < len; i++) {
    z += mod(m[i] + n[i], 2);
  }
  return z;
}
