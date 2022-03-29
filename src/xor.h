#include <stdint.h>

uint8_t* xor_hex(uint8_t* lhs, int l_len, uint8_t* rhs, int r_len);
uint8_t* xor_binary(uint8_t* lhs, uint8_t* rhs, int len);

uint8_t* xor_text_with_key(uint8_t* plaintext, char* key);
