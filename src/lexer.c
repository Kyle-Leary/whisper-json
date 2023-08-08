#include "lexer.h"
#include "defines.h"
#include "types.h"
#include "util.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PEEK (l->text[l->pos + 1])
// macros to bump the lexer pointers up by a specific amount, with error
// handling.
#define BRK_NEXT(num_bumps)                                                    \
  {                                                                            \
    l->pos += num_bumps;                                                       \
    if (l->pos >= INPUT_LEN) {                                                 \
      break;                                                                   \
    }                                                                          \
    l->curr_char = l->text[l->pos];                                            \
  }
#define RET_NEXT(num_bumps)                                                    \
  {                                                                            \
    l->pos += num_bumps;                                                       \
    if (l->pos >= INPUT_LEN) {                                                 \
      return;                                                                  \
    }                                                                          \
    l->curr_char = l->text[l->pos];                                            \
  }
// dumb
#define RET_TOKEN_NEXT(num_bumps)                                              \
  {                                                                            \
    l->pos += num_bumps;                                                       \
    if (l->pos >= INPUT_LEN) {                                                 \
      return (Token){EMPTY, value};                                            \
    }                                                                          \
    l->curr_char = l->text[l->pos];                                            \
  }

// this is only for bumping the token internally through the next() function.
// doesn't need to be exposed?
// token_at_cursor leaves the lexer state pointing to the last character in the
// lexeme. for example, if the lexeme parsed out is 'a', the cursor will point
// to the last ' in the literal decl after returning from the token_at_cursor
// method.
static Token token_at_cursor(Lexer *l) {
  // we have our own special definition of NEXT for this function and the other
  // one. this one break;s instead of return;ing
  Lexeme l_type = LEXEME_NULL;
  TokenValue value = NO_TOKEN_DATA;

  // we can logically group lexemes into one-character and multi-character ones.
  char ch = l->curr_char;

  if (ch == 0) {
    return (Token){EMPTY, NO_TOKEN_DATA};
  }

  // JSON doesn't have char literals.
  if (l->curr_char == '\"') { // PARSE STRING LITERAL
    int sz = 0;

    l_type = STRING_LITERAL;
    RET_TOKEN_NEXT(1); // bump past the first " in the string literal, into the
                       // real string.

    char literal_buf[MAX_STR_LITERAL_SIZE];

    char literal_ch = l->curr_char;
    while (literal_ch != '\"') { // until it hits the other "
      if (sz >= MAX_STR_LITERAL_SIZE) {
        while (literal_ch != '\"') { // if we overflow the max literal size,
                                     // still keep reading out the literal, just
                                     // don't save it to the buffer or the size.
          RET_TOKEN_NEXT(1);
          literal_ch = l->curr_char;
        }
        break;
      }

      literal_buf[sz] = literal_ch;
      sz++;
      RET_TOKEN_NEXT(
          1); // then, bump the pointer and read from the new literal value.
      literal_ch = l->curr_char;
    }

    char *temp_value = (char *)malloc(
        sz + 1);           // malloc the size, then copy the buffer right in.
    temp_value[sz] = '\0'; // null term all strings?

    memcpy(temp_value, literal_buf, sz);
    value = (TokenValue){
        .as_ptr = temp_value}; // then, return the raw pointer to the string as
                               // the token value, so that everything else can
                               // easily access it through the ast or whatever.

  } else if (isdigit(l->curr_char)) { // parse numeric literals, starting with
                                      // INT_LITERAL.

    l_type = NUMERIC_LITERAL; // the type doesn't actually matter, they all just
                              // hold a double of data anyway.

    char literal_buf[MAX_STR_LITERAL_SIZE];

    int i = 0;

    char literal_ch = l->curr_char;
    while (isdigit(literal_ch)) {
      if (i >= MAX_STR_LITERAL_SIZE) {
        while (isdigit(literal_ch)) {
          RET_TOKEN_NEXT(1);
          literal_ch = l->curr_char;
        }
        break;
      }

      literal_buf[i] = literal_ch;
      i++;
      RET_TOKEN_NEXT(
          1); // then, bump the pointer and read from the new literal value.
      literal_ch = l->curr_char;
    }

    RET_TOKEN_NEXT(-1);

    // null term, then convert the number.
    literal_buf[i] = '\0';
    value.as_uint = atoi(literal_buf);
  } else {
    switch (ch) {
    case '[':
      l_type = LSQUARE;
      break;
    case ']':
      l_type = RSQUARE;
      break;
    case '{':
      l_type = LCURLY;
      break;
    case '}':
      l_type = RCURLY;
      break;
    case ':':
      l_type = COLON;
      break;
    case ',':
      l_type = COMMA;
      break;
    case '\"':
      l_type = DOUBLE_QUOTE;
      break;
    default: {
      // now, handle things that aren't just simple one-character lexemes.
      // using strlen later, need to zero-alloc this.
      char keyword_buf[MAX_KW_LEN] = {0};
      int i = 0;

      // then, try to parse out keywords if everything else fails.
      while (isalnum(l->curr_char) ||
             l->curr_char == '_') { // while it's a valid character that can
                                    // appear in a keyword or identifier:
        if (i >= MAX_KW_LEN)        // don't overrun the keyword buffer either.
          break;
        keyword_buf[i] =
            l->curr_char; // modify the lexer state directly in the next
                          // function, it's required. the keyword functions
                          // AREN'T a lookahead, they actually move the state
                          // itself ahead.
        i++;
        BRK_NEXT(1); // will automatically break if we overrun the Lexer text
                     // buffer.
      }

      BRK_NEXT(-1);

      if (strncmp(keyword_buf, "null", 4) == 0) {
        l_type = KW_NULL;
      } else if (strncmp(keyword_buf, "true", 4) == 0) {
        l_type = KW_TRUE;
      } else if (strncmp(keyword_buf, "false", 5) == 0) {
        l_type = KW_FALSE;

      } else {
        dump_lexer(l);
        error("ERROR: could not find a valid character at the lexer cursor, "
              "not even a keyword. character found: %c\n",
              l->curr_char);
      }
    } break;
    }
  }

  return (Token){l_type, value};
}

// handle whitespace skipping and comment skipping, right here in the lexer.
void next(Lexer *l) {
  // get ready for some crazy shit
  // just align to the next character in the lexer.

begin_next:

  RET_NEXT(1);

  // try to avoid calling the lexeme_from_char full lexing function if we
  // don't have to, like in the case of comments and whitespace.
  if ((l->curr_char == ' ') || (l->curr_char == '\n') ||
      (l->curr_char == '\t')) {
    goto begin_next; // FUCK recursion.
  }

  l->curr_token = token_at_cursor(l);
}

void eat(Lexer *lx, Lexeme l) {
  if (lx->curr_token.type != l) {
    dump_lexer(lx);
    error("Eat error: lexemes did not match - Your \"%s\" vs the lexer's "
          "\"%s\".",
          lexeme_to_string(l), lexeme_to_string(lx->curr_token.type));
  }
  next(lx);
}

// returns a statically allocated read-only string literal in the program's
// data section, that's why we can safely return a const char* here and use it
// in the caller without mallocing anything.
const char *lexeme_to_string(Lexeme lexeme) {
  switch (lexeme) {
  case LEXEME_NULL:
    return "LEXEME_NULL";
  case NUMERIC_LITERAL:
    return "NUMERIC_LITERAL";
  case STRING_LITERAL:
    return "STRING_LITERAL";
  case LSQUARE:
    return "LSQUARE";
  case RSQUARE:
    return "RSQUARE";
  case LCURLY:
    return "LCURLY";
  case RCURLY:
    return "RCURLY";
  case COMMA:
    return "COMMA";
  case COLON:
    return "COLON";
  case DOUBLE_QUOTE:
    return "DOUBLE_QUOTE";
  case WHITESPACE:
    return "WHITESPACE";
  case EMPTY:
    return "EMPTY";
  case KW_NULL:
    return "KW_NULL";
  case KW_FALSE:
    return "KW_FALSE";
  case KW_TRUE:
    return "KW_TRUE";
  default:
    return "UNKNOWN_LEXEME";
  }
}

bool is_keyword(Lexeme l) { return (l >= KEYWORD_START && l <= KEYWORD_END); }

void dump_lexer(Lexer *l) {
  printf("---BEGIN LEXER DUMP---\n");
#define TEXT_DUMP_LEN 12
  // for (int i = 0; i < TEXT_DUMP_LEN; i++) {
  //   // where i is an offset into the text position pointer.
  //   int offset_position = l->pos + i - 6;
  //   // print a "you are here" type indicator.
  //   printf("%s", (offset_position == l->pos) ? ">" : " ");
  // }
  //
  // printf("\n");

#define REDCHAR(ch) printf("\033[41m%c\033[0m", ch)

  printf("You are at ");
  REDCHAR(' ');
  printf(":\n");

  for (int i = 0; i < TEXT_DUMP_LEN; i++) {
    int offset_position = l->pos + i - 6;

    char curr_ch = l->text[offset_position];

    if (offset_position == l->pos) {
      // TODO: maybe use ansi codes to give the character a nice background?
      REDCHAR(curr_ch);
    } else if (offset_position < l->text_len) {
      printf("%c", curr_ch);
    } else {
      printf("?");
    }
  }
#undef TEXT_DUMP_LEN

  printf("\n");

  printf("curr position into text: %d\n", l->pos);
  printf("curr token lexeme type: %d\n", l->curr_token.type);
  printf("curr token value: %lu\n", l->curr_token.value.as_uint);
  printf("curr token lexeme type string: %s\n",
         lexeme_to_string(l->curr_token.type));
  printf("---END LEXER DUMP---\n");
}

// don't expose the static methods, just expose one testing method for the
// main testing function to call.
void test_lexer() {
  // re-clear the lexer and copy in the string literal.
#define SETUP_LEX(text_input_literal)                                          \
  {                                                                            \
    memset(l, 0, sizeof(Lexer));                                               \
    const char *text_input = text_input_literal;                               \
    l->text_len = strlen(text_input);                                          \
    l->pos = -1;                                                               \
    l->curr_token.type = EMPTY;                                                \
    memcpy(l->text, text_input, l->text_len);                                  \
  }

  printf("\nBEGIN LEXER TESTING:\n\n\n");

  Lexer *l = (Lexer *)malloc(sizeof(Lexer) * 1);

  printf("\n\nEND LEXER TESTING.\n");

  free(l);
#undef SETUP_LEX
}

// these macros use locally-scoped function variables, and aren't useful outside
// of here anyway.
#undef PEEK
#undef BRK_NEXT
#undef RET_NEXT
#undef RET_TOKEN_NEXT
