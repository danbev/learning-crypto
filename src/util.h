#include <stdlib.h>
#include <stdint.h>


uint8_t* hex_to_binary(uint8_t* hex);
int binary_len_of_hex(uint8_t* hex);

void dec_to_bin(int n, int* b, int len);

/* 
 * Converts hexadecimal to base64.
 */
char* hex_to_base64(uint8_t* hex);

void print_hex(uint8_t* hex);
void print_binary(uint8_t* b, int len);


uint8_t* bin_to_hex(uint8_t* bin, int len);
int hex_array_len(uint8_t* arr);

int nr_of_bits_dec(int n);
int nr_of_bits_hex(int n);

uint8_t dec_to_hex(int dec);

uint8_t* xor_hex(uint8_t* lhs, int l_len, uint8_t* rhs, int r_len);
uint8_t* xor_binary(uint8_t* lhs, uint8_t* rhs, int len);
