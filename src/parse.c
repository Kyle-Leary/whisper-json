#include "parse.h"

#include "defines.h"
#include "json_object.h"
#include "lexer.h"
#include "util.h"
#include "wjson.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define MAX_ARRAY_LEN 256

static WJSONValue *value(Lexer *l);

static WJSONValue *decl_list(Lexer *l) {
  // just inline the DECL rule so that we don't have to make a structure and
  // pass around both the key and value together.
#define INSERT_DECL(object)                                                    \
  char *key = l->curr_token.value.as_ptr;                                      \
  eat(l, STRING_LITERAL);                                                      \
  eat(l, COLON);                                                               \
  wjson_object_insert(object, key, value(l));

  WJSONObject obj = wjson_object_create();

  // hack to support empty arrays, since we can't really insert an EMPTY value
  // without making things ugly in the API.
  if (l->curr_token.type == RCURLY)
    goto end_object_builder;

  INSERT_DECL(obj)
  while (l->curr_token.type == COMMA) {
    eat(l, COMMA);
    INSERT_DECL(obj)
  }

end_object_builder :

{ // haha clangd goes apeshit around labels but it's fine if we wrap this in a
  // block.
  WJSONValue *final_array = (WJSONValue *)malloc(sizeof(WJSONValue));
  final_array->data.value.object = obj;
  final_array->type = WJ_TYPE_OBJECT;

  return final_array;
}

#undef INSERT_DECL
}

static WJSONValue *object(Lexer *l) {
  eat(l, LCURLY);
  WJSONValue *inner_list = decl_list(l);
  eat(l, RCURLY);
  return inner_list;
}

// parse the inner bits of an array.
static WJSONValue *value_list(Lexer *l) {
#define APPEND                                                                 \
  memcpy(list_values + len, value(l), sizeof(WJSONValue));                     \
  len++;

  // very similar to object constructor, see decl_list above.
  if (l->curr_token.type == RSQUARE)
    goto end_array_builder;

  int len = 0;
  // we don't know how long the malloced array in the actual structure needs to
  // be at first, so do this in a big temporary buffer and copy over into an
  // array ptr at the end of the function.
  WJSONValue list_values[MAX_ARRAY_LEN];
  APPEND

  while (l->curr_token.type == COMMA) {
    eat(l, COMMA);
    APPEND
  }

end_array_builder :

{
  WJSONValue *final_array = (WJSONValue *)malloc(sizeof(WJSONValue));

  int array_sz = sizeof(WJSONValue) * len;
  WJSONValue *final_array_ptr = (WJSONValue *)malloc(array_sz);
  memcpy(final_array_ptr, list_values,
         array_sz); // if len is greater than the MAX_ARRAY_LEN allocated
                    // buffer, then... uh...

  final_array->data.value.array = final_array_ptr;
  final_array->data.length.array_len = len;
  final_array->type = WJ_TYPE_ARRAY;

  return final_array;
}
#undef APPEND
}

static WJSONValue *array(Lexer *l) {
  eat(l, LSQUARE);
  WJSONValue *inner_list = value_list(l);
  eat(l, RSQUARE);
  return inner_list;
}

static WJSONValue *value(Lexer *l) {
  Lexeme cl = l->curr_token.type;

  WJSONValue *json_value = (WJSONValue *)calloc(sizeof(WJSONValue), 1);

  if (cl == KW_NULL) {
    json_value->type = WJ_TYPE_NULL;
    eat(l, KW_NULL);
  } else if (cl == KW_FALSE) {
    json_value->type = WJ_TYPE_BOOLEAN;
    json_value->data.value.boolean = false;
    eat(l, KW_FALSE);
  } else if (cl == KW_TRUE) {
    json_value->type = WJ_TYPE_BOOLEAN;
    json_value->data.value.boolean = true;
    eat(l, KW_TRUE);
  } else if (cl == STRING_LITERAL) {
    json_value->type = WJ_TYPE_STRING; // all strings are double-quoted.
    json_value->data.length.str_len = strlen(l->curr_token.value.as_ptr);
    // actually allocate the string. assure that all inner values will always be
    // valid.
    json_value->data.value.string =
        (char *)malloc(json_value->data.length.str_len);
    memcpy(json_value->data.value.string, l->curr_token.value.as_ptr,
           json_value->data.length.str_len);
    eat(l, STRING_LITERAL);
  } else if (cl == NUMERIC_LITERAL) {
    json_value->type = WJ_TYPE_NUMBER;
    // TODO: need to make the token literal value a double by default here.
    json_value->data.value.number = l->curr_token.value.as_double;
    eat(l, NUMERIC_LITERAL);
  } else if (cl == LCURLY) {
    json_value = object(l);
  } else if (cl == LSQUARE) {
    json_value = array(l);
  } else if (cl == COMMA) {
    error("Blank values are not allowed, found a COMMA instead of a "
          "proper value when trying to parse a value.\n");
  } else if (cl == EMPTY) {
    dump_lexer(l);
    error("Reached EOF when trying to parse a JSON value.\n");
  } else {
    dump_lexer(l);
    error("Invalid starting Lexeme for value. [lexeme id %d] [lexeme rep "
          "%s]\n",
          cl, lexeme_to_string(cl));
  }

  return json_value;
}

// parse is the only non-static API method of the parser. don't even bother with
// an AST for something as simple as parsing JSON. just directly return a
// JSONValue from each rule, and go from there, recursively parsing until we're
// done with the toplevel value.
WJSONValue *parse(char text_input[INPUT_LEN]) {
  Lexer *l = (Lexer *)malloc(
      sizeof(Lexer)); // pass through the lexer manually and malloc that. TODO:
                      // is there a better way to place the lexer in memory?

  // use the actual string length and not the buffer length to do the EOF
  // check.
  l->text_len = strlen(text_input);
  l->pos = -1; // set to -1, the init next() call will put it at 0.
  l->curr_token.type = EMPTY;

  memcpy(l->text, text_input, INPUT_LEN);
  next(l);

  // values all the way down in JSON, all starting from one root value.
  return value(l);
}

void test_parse() {
  printf("\n\nBEGIN PARSER TESTING\n\n\n");

  printf("\n\nEND PARSER TESTING.\n\n\n");
}
