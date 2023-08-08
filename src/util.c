#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  fprintf(stderr, "Error: ");
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  va_end(args);
  exit(1);
}

int char_to_int(char digit) {
  printf("Converting %c to an int.\n", digit);
  if (digit >= '0' && digit <= '9') {
    return digit - '0';
  }
  error("Digit conversion: failed to convert char to digit.\n");
  return -1;
}

size_t read_into_buf(const char *path, char *buffer, size_t bufferSize) {
  FILE *file = fopen(path, "rb");
  if (file == NULL) {
    char error_buf[256];
    sprintf(error_buf, "Error opening file [%s]", path);
    perror(error_buf);
    exit(1);
    return 0;
  }

  size_t bytesRead = fread(buffer, sizeof(char), bufferSize, file);
  if (ferror(file)) {
    char error_buf[256];
    sprintf(error_buf, "Error reading from file [%s]", path);
    perror(error_buf);
    exit(1);
    bytesRead = 0;
  }

  fclose(file);
  return bytesRead;
}
