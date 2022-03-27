#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

int main(int argc, char** argv) {
  printf("Cryptopals Set 1, Challenge 2 XOR\n");

  uint8_t lhs[] = "1c0111001f010100061a024b53535009181c";
  uint8_t rhs[] = "686974207468652062756c6c277320657965";
  uint8_t* expected = {"746865206b696420646f6e277420706c6179"};
  
  printf("Input 1: ");
  print_hex(lhs);
  printf("Input 2: ");
  print_hex(rhs);

  uint8_t* xored = xor_hex(lhs, strlen(lhs), rhs, strlen(rhs)); 
  //print_binary(xored, hex_array_len(lhs));

  uint8_t* actual = bin_to_hex(xored, hex_array_len(lhs));
  printf("Expected :%s\n", expected);
  printf("Actual   :");
  print_hex(actual);
}
