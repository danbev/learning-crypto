#define _GNU_SOURCE
#include <stdio.h>

int main(int argc, char** argv) {
  printf("Cryptopals Set 1, challenge 6\n");

  char* filename = "src/6.txt";
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    printf("Could not locate file: %s\n", filename);
    return -1;
  }
  char* line = NULL;
  size_t len = 0;
  int read = 0;
  while (((read = getline(&line, &len, file)) != -1)) {
    if (line[read-1] == '\n') {
      line[read-1] = '\0';
    }
    printf("%s\n", line);
  }

  fclose(file);

  return 0;
}
