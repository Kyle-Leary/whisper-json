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
    WJSONValue *value = wjson_parse_file("tests/bool.json");
    assert(value->type == WJ_TYPE_BOOLEAN);
    assert(value->data.value.boolean == true);
    wjson_pretty_print(value);
  }

  { // do the same thing, but test the input_buffer version directly.
    WJSONValue *value = wjson_parse_string("false");
    assert(value->type == WJ_TYPE_BOOLEAN);
    assert(value->data.value.boolean == false);
    wjson_pretty_print(value);
  }

  {
    WJSONValue *value = wjson_parse_file("tests/number.json");
    assert(value->type == WJ_TYPE_NUMBER);
    assert(value->data.value.number == 5);
    wjson_pretty_print(value);
  }

  {
    WJSONValue *value = wjson_parse_file("tests/string.json");
    assert(value->type == WJ_TYPE_STRING);
    assert(strncmp(value->data.value.string,
                   "top level strings are valid JSON.",
                   value->data.length.str_len) == 0);
    wjson_pretty_print(value);
  }

  {
    WJSONValue *value = wjson_parse_file("tests/null.json");
    assert(value->type == WJ_TYPE_NULL);
    wjson_pretty_print(value);
  }

  // then, test larger objects.

  { // grabbing string fields from a top level object
    WJSONValue *value = wjson_parse_file("tests/basic.json");
    WJSONValue *name = wjson_get(value, "name");
    assert(strncmp(name->data.value.string, "John",
                   name->data.length.str_len) == 0);
    wjson_pretty_print(value);
  }

  { // grabbing an array from an object, then indexing a string into that array.
    WJSONValue *value = wjson_parse_file("tests/nesting.json");
    WJSONValue *skills_arr = wjson_get(value, "skills");
    WJSONValue *first_skill = wjson_index(skills_arr, 0);
    assert(strncmp(first_skill->data.value.string, "Java",
                   first_skill->data.length.str_len) == 0);
    wjson_pretty_print(value);
  }

  { // show off prettyprinting a little, it works well with arrays.
    WJSONValue *value = wjson_parse_file("tests/arrays.json");
    wjson_pretty_print(value);
  }

  return 0;
}
