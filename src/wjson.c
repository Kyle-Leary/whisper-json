#include "wjson.h"
#include "json_object.h"
#include "parse.h"
#include "util.h"
#include <stdio.h>

// define the main api endpoint functions in this.

// wrapper around parse_string.
WJSONValue *wjson_parse_file(const char *file_path) {
  char input_buffer[INPUT_LEN] = {0};
  read_into_buf(file_path, input_buffer, INPUT_LEN);
  return wjson_parse_string(input_buffer);
}

WJSONValue *wjson_parse_string(char *input) {
  WJSONValue *root_value = parse((char *)input);

  // why even bother having a wrapper type, just return the data directly.
  return root_value;
}

static void wjson_pretty_print_recursive(WJSONValue *value, int indent_level,
                                         int show_hash) {
#define INDENT                                                                 \
  for (int i = 0; i < indent_level; i++) {                                     \
    printf("  ");                                                              \
  }

  INDENT

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
                                   indent_level + 1, show_hash);
      if (i < value->data.length.array_len - 1) {
        printf(",");
      }
      printf("\n");
    }

    INDENT

    printf("]");
    break;
  case WJ_TYPE_OBJECT:
    // this kind of sucks. it just prints all the object values that aren't
    // NULL. we have no way of knowing the actual key without massive overhead,
    // so just print the final hash of the key instead.
    printf("{\n");

    for (unsigned int i = 0; i < WJSON_OBJECT_LEN; i++) {
      WJSONValue *curr_value = &value->data.value.object[i];
      if (curr_value->type == WJ_TYPE_INVALID)
        continue;
      else {
        wjson_pretty_print_recursive(curr_value, indent_level + 1, show_hash);
        if (i < value->data.length.array_len - 1) {
          if (show_hash)
            printf(" (hash: %d),", i);
          else
            printf(" ,");
        }
        printf("\n");
      }
    }

    INDENT

    printf("}");
    break;
  case WJ_TYPE_NULL:
    printf("null");
    break;
  default:
    printf("Unknown type");
    break;
  }
}

void wjson_pretty_print(WJSONValue *value, int show_hash) {
  wjson_pretty_print_recursive(value, 0, show_hash);
  printf("\n");
}
