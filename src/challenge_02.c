#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "hex.h"
#include "xor.h"
#include "bin.h"

int main(int argc, char** argv) {
  printf("Cryptopals Set 1, Challenge 2 XOR\n");

  uint8_t lhs[] = "1c0111001f010100061a024b53535009181c";
  uint8_t rhs[] = "686974207468652062756c6c277320657965";
  uint8_t* expected = {"746865206b696420646f6e277420706c6179"};
  
  printf("Input 1: %s\n", lhs);
  printf("Input 2: %s\n", rhs);

  uint8_t* xored = xor_hex(lhs, strlen(lhs), rhs, strlen(rhs)); 

  uint8_t* actual = bin_to_hex(xored, binary_len_of_hex(lhs));
  printf("Expected :%s\n", expected);
  printf("Actual   :");
  print_hex(actual, strlen(lhs));
}
