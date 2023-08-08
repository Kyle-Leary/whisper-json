#include "wjson.h"
#include "parse.h"
#include "util.h"
#include <stdio.h>

// define the main api endpoint functions in this.

WJSONValue *wjson_parse_file(const char *file_path) {
  char input_buffer[INPUT_LEN] = {0};
  read_into_buf(file_path, input_buffer, INPUT_LEN);

  WJSONValue *root_value = parse(input_buffer);

  // why even bother having a wrapper type, just return the data directly.
  return root_value;
}
