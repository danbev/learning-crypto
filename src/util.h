#include <stdlib.h>
#include <stdint.h>

int nr_of_bits_dec(int n);

void dec_to_binary(int n, int* b, int len);
void print_binary(uint8_t* b, int len);

uint8_t* bin_to_hex(uint8_t* bin, int len);

uint8_t dec_to_hex(int dec);

uint8_t* xor_hex(uint8_t* lhs, int l_len, uint8_t* rhs, int r_len);
uint8_t* xor_binary(uint8_t* lhs, uint8_t* rhs, int len);
