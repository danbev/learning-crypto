#include <math.h>

int mod(int a, int n) {
  int r = a - n * floor(a / n);
  return (r < 0) ? r + n : r;
}
