#include "lexer.h"
#include "parse.h"
#include "wjson.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
// a testing entry point. wjson is a library, but we need to test somehow.

int main(int argc, char *argv[]) {
  // test individual segments of the program.
  test_parse();
  test_lexer();

  // test examples of all the basic types at top-level.
  {
    WJSONFile value = wjson_parse_file("tests/bool.json");
    assert(value.root->type == WJ_TYPE_BOOLEAN);
    assert(value.root->data.value.boolean == true);
  }

  {
    WJSONFile value = wjson_parse_file("tests/number.json");
    assert(value.root->type == WJ_TYPE_NUMBER);
    assert(value.root->data.value.number == 5);
  }

  {
    WJSONFile value = wjson_parse_file("tests/string.json");
    assert(value.root->type == WJ_TYPE_STRING);
    assert(strncmp(value.root->data.value.string,
                   "top level strings are valid JSON.",
                   value.root->data.length.str_len) == 0);
  }

  {
    WJSONFile value = wjson_parse_file("tests/null.json");
    assert(value.root->type == WJ_TYPE_NULL);
  }

  // then, test larger objects.

  wjson_parse_file("tests/basic.json");
  return 0;
}
