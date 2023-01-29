#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vigenere.h"

int main(int argc, char** argv) {
  char* plaintext = NULL;
  char* keyword = NULL;

  if (argc == 3) {
    plaintext = argv[1];
    keyword = argv[2];
  } else if (argc == 2) {
    char* line = NULL;
    size_t len = 0;
    int read = 0;

    int size = sizeof(char) * 2048;
    plaintext = malloc(size);
    int ch;
    int count = 0;
    while (((read = getline(&line, &len, stdin)) != -1)) {
      //printf("count: %d\n", count);
      if (count+read > size) {
        printf("going to realloc...\n");
        plaintext = realloc(plaintext, size+size);
        size += size;
      }
      memcpy(plaintext+count, line, read);
      count += read;
    }
    *(plaintext+count-1) = '\0';
    free(line);
    //printf("count: %d\n", count);
    //printf("%s\n", plaintext);
    keyword = argv[1];
  } else {
    printf("Usage: vigenere_encrypt <plaintext> <keyword>\n");
    exit(-1);
  }

  printf("keyword: %s\n", keyword);
  char* ciphertext = vigenere_encrypt(plaintext, keyword);
  printf("ciphertext: %s\n", ciphertext);

  return 0;
}
