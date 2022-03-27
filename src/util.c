#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

void print_binary(uint8_t* b, int size) {
  for (int i = 0; i < size; i++) {
    printf("%d", b[i]);
  }
  printf("\n");
}

void print_hex(uint8_t* hex) {
  int len = strlen(hex);
  for (int i = 0; i < len; i++) {
    printf("%c", hex[i]);
  }
  printf("\n");
}

void dec_to_binary(int n, int* b, int len) {
  int x = n;
  for (int i = len - 1; i >= 0; i--) {
    int r = x % 2;
    x /= 2;
    b[i] = r;
  }
}

int binary_len_of_hex(uint8_t* hex) {
  int len = strlen(hex);
  return len * 4;
}

uint8_t* hex_to_binary(uint8_t* hex) {
  int len = strlen(hex);
  int b_len = len * 4;
  uint8_t* arr = (uint8_t*) malloc(b_len);

  for (int i = 0, k = b_len, j = 0; i < len; i++) {
    int dec = 0;
    if (hex[i] >= 48 && hex[i] <= 57) {
      dec += hex[i] - 48;
    } else if (hex[i] >= 65 && hex[i] <= 70) {
      dec += hex[i] - 55;
    } else if (hex[i] >= 97 && hex[i] <= 103) {
      dec += hex[i] - 87;
    }

    int idx = i * 4 + 3;
    int q = dec;
    while (q > 0) {
      int r = q % 2;
      q = q / 2;
      arr[idx--] = r;
    }
  }
  return arr;
}

int bin_to_dec(uint8_t* b, int len) {
  int dec = 0;
  for (int i = 0, j = len-1; i < len; i++, j--) { 
    dec += (int) pow(2, j) * b[i];
  } 
  return dec;
}

uint8_t* bin_to_hex(uint8_t* bin, int len) {
  int h_len = len/4;
  uint8_t* hex = (uint8_t*) malloc(h_len);

  uint8_t nibble[4];
  int idx = 0;
  for (int i = 0, j=0, idx = 0; j < h_len; idx+=4, j++ ) {
    nibble[0] = bin[i++];
    nibble[1] = bin[i++];
    nibble[2] = bin[i++];
    nibble[3] = bin[i++];
    int d = bin_to_dec(nibble, 4);

    int r = d % 16;
    d = d / 16;
    if (r > 9) {
      hex[j] += r + 55;
    } else {
      hex[j] = r + 48;
    }
    //printf("j: %d, %d %d %d %d = %c\n", j, nibble[0], nibble[1], nibble[2], nibble[3], hex[j]);
  }
  return hex;
}

int _log(int base, int x) {
  int power = 0;
  while((x /= base) >= 1) {
    power++;
  }
  return power;
}

int factorial(int n) {
  int f = 1;
  for (int i=1; i <= n; i++) {
    f *= i;
  }
  return f;
}

int power(int x, int n) {
  int r = 1;
  for (int i = 0; i < n; i++) {
    r *= x;
  }
  return r;
}

int square_root(int x) {
  if (x < 2) {
    return x;
  }

  int r;
  int start = 1;
  int end = x/2;
  while (start <= end) {
    int mid = (start+end)/2;
    long square = mid*mid;
    if (square == x) {
      return mid;
    }
    if (square < x) {
      start = mid+1;
      r = mid;
    } else {
      end = mid-1;
    }
  }
  return r;
}

int nr_of_bits_dec(int n) {
  return floor(log(n)/log(2)) + 1;
}

int nr_of_bits_hex(int n) {
  return floor(log(n)/log(16)) + 1;
}

uint8_t* dec_to_hex_malloc(int n) {
  int q = n;
  int len = nr_of_bits_hex(n);
  uint8_t* arr = (uint8_t*) malloc(len);
  for (int i = len-1; q > 0; i--) {
    int r = q % 16;
    q = q / 16;
    if (r > 9) {
      arr[i] += r + 55;
    } else {
      arr[i] = r + 49;
    }
  }
  return arr;
}


uint8_t* xor_hex(uint8_t* lhs, int l_len, uint8_t* rhs, int r_len) {
  int b_len = l_len * 4;
  uint8_t* lhs_binary = hex_to_binary(lhs);
  //print_binary(lhs_binary, b_len);

  uint8_t* rhs_binary = hex_to_binary(rhs);
  //print_binary(rhs_binary, b_len);

  uint8_t* xored = (uint8_t*) malloc(b_len);
  for (int i = 0; i < b_len; i++) {
    xored[i] = lhs_binary[i] ^ rhs_binary[i];
  }
  return xored;
}

uint8_t* xor_binary(uint8_t* lhs, uint8_t* rhs, int len) {
  //print_binary(lhs, len);
  //print_binary(rhs, len);

  uint8_t* xored = (uint8_t*) malloc(len);
  for (int i = 0; i < len; i++) {
    xored[i] = lhs[i] ^ rhs[i];
  }
  return xored;
}
