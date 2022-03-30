#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "xor.h"
#include "hex.h"
#include "bin.h"
#include "str.h"

/*
 * Challenge: https://cryptopals.com/sets/1/challenges/5
 */
int main(int argc, char** argv) {
  printf("Cryptopals Set 1, Challenge 5\n");
  char* plaintext = "Burning 'em, if you ain't quick and nimble\nI go crazy when I hear a cymbal";
  char* key = "ICE";
  char* expected = "0b3637272a2b2e63622c2e69692a23693a2a3c6324202d623d63343c2a26226324272765272a282b2f20430a652e2c652a3124333a653e2b2027630c692b20283165286326302e27282f";

  printf("plain_text:\n%s\n", plaintext);
  printf("key: %s\n", key);

  uint8_t* xored = xor_text_with_key(plaintext, key);
  int b_len = binary_len_of_str(plaintext);
  uint8_t* xored_hex = bin_to_hex(xored, b_len);
  print_hex(expected);
  print_hex(xored_hex);
  assert(strcmp(xored_hex, expected) == 0);
}
