#pragma once

#include "defines.h"
#include <stdbool.h>
#include <stdint.h>

// specifically define ranges on the lexer enum table.
#define KEYWORD_START 0b100000000000
#define KEYWORD_END 0b111111111111

// Lexeme is the type, token is the instance of the specific lexeme produced by
// the lexer.
typedef enum Lexeme {
  LEXEME_NULL = 0, // why not

  NUMERIC_LITERAL, // literal type lexemes.
  STRING_LITERAL,  // double quote string

  ID, // an ID for a variable or function or whatnot, parsed as a
      // backup if the keyword parsing falls through.

  ADD,
  SUB,
  MUL,
  DIV,

  LPAREN, // ()
  RPAREN,
  LSQUARE, // []
  RSQUARE,
  LCURLY, // {}
  RCURLY,

  SEMI,

  COMMA,
  COLON,

  EQUAL,

  SINGLE_QUOTE,
  DOUBLE_QUOTE,

  WHITESPACE, // newline and spaces
  EMPTY,      // this would be EOF, but that appears to be reserved.

  // keyword defs
  KW_NULL = KEYWORD_START,
  KW_FALSE,
  KW_TRUE,
} Lexeme;

typedef Value64 TokenValue;

#define NO_TOKEN_DATA ((Value64){.as_uint = 0})

// what the lexeme actually holds internally.
typedef struct Token {
  Lexeme type; // store the type of the token and some arbitrary data value,
               // either just a u64 or an actual pointer.
  TokenValue value;
} Token;

// ideally, the lexer is the only one with direct access to the text.
// it crunches that into Lexemes -> tokens, and it's all high-level from there
// on out.
typedef struct Lexer {
  char text[INPUT_LEN]; // the current text input it's crunching through.
  int text_len;
  int pos;          // index into the text input of the lexer.
  char curr_char;   // the raw character at the current position.
  Token curr_token; // the current token the parser is on.
} Lexer;

// debug dumping of the whole Lexer from a pointer.
void dump_lexer(Lexer *l);

int get_int(Lexer *l);
void next(Lexer *l);
void eat(Lexer *lx, Lexeme l);

const char *lexeme_to_string(Lexeme lexeme);

// simple comparison wrapper functions around the freaky enum start/end stuff.
bool is_keyword(Lexeme l);
bool is_datatype_keyword(
    Lexeme l); // is it one of the datatype keywords, like KW_INT?

void test_lexer();
