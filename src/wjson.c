#include "wjson.h"
#include "ast.h"
#include "parse.h"
#include "util.h"
#include "visit.h"
#include <stdio.h>

// define the main api endpoint functions in this.

// lex -> parse -> visit AST -> create API JSON structure for the client to
// traverse -> done!
WJSONFile wjson_parse_file(const char *file_path) {
  // use a file as input for the compiler.
  char input_buffer[INPUT_LEN] = {0};
  read_into_buf(file_path, input_buffer, INPUT_LEN);

  NodeIndex root_node = parse(input_buffer);
  WJSONValue *root_value = value_from_root(root_node);

  // clean all the nodes, get ready for another parsing. we don't want to fill
  // up the tree and not have any more room.
  clean_ast();

  return (WJSONFile){.root = root_value};
}
