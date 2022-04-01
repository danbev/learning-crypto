#include <stdint.h>

char* hex_to_base64(uint8_t* hex);
uint8_t* base64_to_hex(char* base64);
uint8_t* base64_to_binary(int n, uint8_t* b);
int binary_len_of_base64(char* b);
