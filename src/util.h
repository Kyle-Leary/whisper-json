#pragma once

#include <stddef.h>

#define ASSERT(condition, message)                                             \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fprintf(stderr, "Assertion failed at %s:%d: %s\n", __FILE__, __LINE__,   \
              message);                                                        \
      exit(EXIT_FAILURE);                                                      \
    } else {                                                                   \
      fprintf(stdout, "Assertion succeeded at %s:%d: %s\n", __FILE__,          \
              __LINE__, message);                                              \
    }                                                                          \
  } while (0)

// make the linter not freak out when we don't return a value after an error.
void error(const char *format, ...) __attribute__((__noreturn__));
int char_to_int(char digit);
size_t read_into_buf(const char *path, char *buffer, size_t bufferSize);
