#include <stdio.h>
#include <stdlib.h>

/*
 * This program is used to print frequencies of English characters.
 *
 * I used The Great Gatsby for this example:
 * https://gutenberg.org/cache/epub/64317/pg64317.txt
 */
int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: freq filename\n");
    return -1;
  }
  char* file_name = argv[1];
  char ch;

  FILE* file = fopen(file_name, "r");
  double lower_case[26] = {0};
  double upper_case[26] = {0};
  int total_lowercase = 0;
  int total_uppercase = 0;
  do {
    ch = fgetc(file);
    if (ch >= 65 && ch <= 90) {
      upper_case[ch - 65]++;
      total_uppercase++;
    } else if (ch >= 97 && ch <= 122) {
      lower_case[ch - 97]++;
      total_lowercase++;
    }
  } while (ch != EOF);
  printf("Total lowercase letters: %d\n", total_lowercase);
  printf("Total uppercase letters: %d\n", total_uppercase);

  printf("Lower case:\n");
  printf("double lowercase[] = {\n");
  for (int i = 0; i < 26; i++) {
    printf("  %f, // %c \n", lower_case[i]/total_lowercase, i+97);
  }
  printf("};\n");

  printf("double uppercase[] = {\n");
  for (int i = 0; i < 26; i++) {
    printf("  %f, // %c \n", upper_case[i]/total_uppercase, i+65);
  }
  printf("};\n");

  fclose(file);

  return 0;
}
