#include "parse.h"

#include "ast.h"
#include "defines.h"
#include "lexer.h"
#include "util.h"
#include "wjson.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// the "do nothing" statement.
static NodeIndex empty(Lexer *l) {
  return add_node(make_node(NT_EMPTY, NULL_INDEX, NULL_INDEX, NO_NODE_DATA));
}

// OR over a bunch of potential statement types.
static NodeIndex value(Lexer *l) {
  Lexeme cl = l->curr_token.type;

  WJSONValue *json_value = (WJSONValue *)calloc(sizeof(WJSONValue), 1);

  if (cl == KW_NULL) {
    json_value->type = WJ_TYPE_NULL;
  } else if (cl == KW_FALSE) {
    json_value->type = WJ_TYPE_BOOLEAN;
    json_value->data.value.boolean = false;
  } else if (cl == KW_TRUE) {
    json_value->type = WJ_TYPE_BOOLEAN;
    json_value->data.value.boolean = true;
  } else if (cl == STRING_LITERAL) {
    json_value->type = WJ_TYPE_STRING; // all strings are double-quoted.
    json_value->data.value.string = l->curr_token.value.as_ptr;
    json_value->data.length.str_len = strlen(l->curr_token.value.as_ptr);
  } else if (cl == NUMERIC_LITERAL) {
    json_value->type = WJ_TYPE_NUMBER;
    // TODO: need to make the token literal value a double by default here.
    json_value->data.value.number = l->curr_token.value.as_uint;
  } else if (cl == LCURLY) {
    // parse dictionary
  } else if (cl == LSQUARE) {
    // parse array
  } else if (cl == COMMA) {
    error("Blank values are not allowed, found a COMMA instead of a "
          "proper value when trying to parse a value.\n");
  } else if (cl == EMPTY) {
    error("Reached EOF when trying to parse a JSON value.\n");
  } else {
    error("Invalid starting Lexeme for value. [lexeme id %d] [lexeme rep "
          "%s]\n",
          cl, lexeme_to_string(cl));
  }

  // parse into a JSONValue directly since it's easy, one less thing the visitor
  // has to do.
  return add_node(make_node(NT_VALUE, NULL_INDEX, NULL_INDEX,
                            (NodeData){.as_ptr = json_value}));
}

// they have seperate grammar rules, but use the generic NT_LIST ast rep.
static NodeIndex value_list(Lexer *l) {
  NodeIndex left = value(l);
  NodeIndex second_term = NULL_INDEX;

  Lexeme cl = l->curr_token.type;

  if (cl == COMMA) {
    second_term = value_list(l);
  }
  // we've reached the end of the args.

  return add_node(make_node(NT_LIST, left, second_term, NO_NODE_DATA));
}

// parse out the key-value pairs in the JSON objects.
static NodeIndex decl(Lexer *l) {
  char *key = l->curr_token.value.as_ptr;

  eat(l, STRING_LITERAL); // "key" : value
  eat(l, COLON);
  NodeIndex value_node = value(l);

  return add_node(
      make_node(NT_DECL, value_node, NULL_INDEX, (NodeData){.as_ptr = key}));
}

static NodeIndex decl_list(Lexer *l) {
  NodeIndex left = decl(l);
  NodeIndex second_term = NULL_INDEX;

  Lexeme cl = l->curr_token.type;

  if (cl == COMMA) {
    second_term = decl_list(l);
  }
  // we've reached the end of the declarations.

  return add_node(make_node(NT_LIST, left, second_term, NO_NODE_DATA));
}

// these are both just list rules with special characters wrapped around them.
static NodeIndex object(Lexer *l) {
  eat(l, LCURLY);
  NodeIndex inner_list = decl_list(l);
  eat(l, RCURLY);
  return inner_list;
}

static NodeIndex array(Lexer *l) {
  eat(l, LSQUARE);
  NodeIndex inner_list = value_list(l);
  eat(l, RSQUARE);
  return inner_list;
}

// parse is the only non-static API method of the parser. it parses the whole
// program into the global AST.
//
// will return the index of the root node into the
// global ast Node array.
NodeIndex parse(char text_input[INPUT_LEN]) {
  Lexer *l = (Lexer *)malloc(
      sizeof(Lexer)); // pass through the lexer manually and malloc that. TODO:
                      // is there a better way to place the lexer in memory?

  // use the actual string length and not the buffer length to do the EOF check.
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
