#include "wjson.h"
#include "parse.h"
#include "util.h"
#include <stdio.h>

// define the main api endpoint functions in this.

// lex -> parse -> visit AST -> create API JSON structure for the client to
// traverse -> done!
WJSONFile wjson_parse_file(const char *file_path) {
  // use a file as input for the compiler.
  char input_buffer[INPUT_LEN] = {0};
  read_into_buf(file_path, input_buffer, INPUT_LEN);

  WJSONValue *root_value = parse(input_buffer);

  return (WJSONFile){.root = root_value};
}
