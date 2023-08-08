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

static void wjson_pretty_print_recursive(WJSONValue *value, int indent_level) {
  // Print indentation
  for (int i = 0; i < indent_level; i++) {
    printf("  ");
  }

  // Handle different types
  switch (value->type) {
  case WJ_TYPE_STRING:
    printf("\"%s\"", value->data.value.string);
    break;
  case WJ_TYPE_NUMBER:
    printf("%f", value->data.value.number);
    break;
  case WJ_TYPE_BOOLEAN:
    printf("%s", value->data.value.boolean ? "true" : "false");
    break;
  case WJ_TYPE_ARRAY:
    printf("[\n");
    for (unsigned int i = 0; i < value->data.length.array_len; i++) {
      wjson_pretty_print_recursive(&value->data.value.array[i],
                                   indent_level + 1);
      if (i < value->data.length.array_len - 1) {
        printf(",");
      }
      printf("\n");
    }
    for (int i = 0; i < indent_level; i++) {
      printf("  ");
    }
    printf("]");
    break;
  case WJ_TYPE_OBJECT:
    printf("{ OBJECT }");
    break;
  case WJ_TYPE_NULL:
    printf("null");
    break;
  default:
    printf("Unknown type");
    break;
  }
}

void wjson_pretty_print(WJSONValue *value) {
  wjson_pretty_print_recursive(value, 0);
  printf("\n");
}
