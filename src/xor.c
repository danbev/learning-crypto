#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "hex.h"
#include "dec.h"
#include "str.h"
#include "xor.h"

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

uint8_t* xor_text_with_key(uint8_t* plaintext, char* key) {
  uint8_t* plaintext_bin = str_to_binary(plaintext);
  int b_len = binary_len_of_str(plaintext);

  uint8_t* key_bin = str_to_binary(key);
  int k_len = binary_len_of_str(key);

  uint8_t* xored = (uint8_t*) malloc(b_len);
  for (int i = 0, j = 0; i < b_len; i++) {
    xored[i] = plaintext_bin[i] ^ key_bin[j++];
    if (j == k_len) {
      j = 0;
    }
  }
  return xored;
}
