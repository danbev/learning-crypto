#include <stdlib.h>
#include <stdint.h>

void print_binary(uint8_t* b, int len);
void print_hex(uint8_t* hex);

void to_binary(int n, int** b, int* len);
void to_hex(int n, uint8_t h, int* h_len);

uint8_t* hex_to_binary(uint8_t* hex);
int binary_len_of_hex(uint8_t* hex);

void append_binary(uint8_t* to, int offset, uint8_t* from, int size);

void dec_to_bin(int n, int* b, int len);
uint8_t* dec_to_bin_malloc(int n);
uint8_t* dec_to_hex_malloc(int n);

int bin_to_dec(uint8_t* b, int len);
uint8_t* bin_to_hex(uint8_t* hex, int len);

char* hex_to_base64(uint8_t* hex);

int nr_of_bits_dec(int n);
int nr_of_bits_hex(int n);

int hex_array_len(uint8_t* arr);
uint8_t dec_to_hex(int dec);

uint8_t* xor(uint8_t* lhs, int l_len, uint8_t* rhs, int r_len);
uint8_t* xor_binary(uint8_t* lhs, uint8_t* rhs, int len);
