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

  printf("Trying to convert one-character lexeme from '%c'.\n", ch);

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

    // then bump the cursor back to the " in the string literal decl.
    RET_TOKEN_NEXT(-1);

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
    // can treat a char like a num and switch over it.
    switch (ch) {
      // first, handle the trivial one-character cases.
    case '+':
      l_type = ADD;
      break;
    case '-':
      l_type = SUB;
      break;
    case '*':
      l_type = MUL;
      break;
    case '/':
      l_type = DIV;
      break;
    case '(':
      l_type = LPAREN;
      break;
    case ')':
      l_type = RPAREN;
      break;
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
    case ';':
      l_type = SEMI;
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

      printf("Finished parsing keyword from the lexer [%s]\n", keyword_buf);

      if (strncmp(keyword_buf, "null", 4) == 0) {
        l_type = KW_NULL;
      } else if (strncmp(keyword_buf, "true", 4) == 0) {
        l_type = KW_TRUE;
      } else if (strncmp(keyword_buf, "false", 5) == 0) {
        l_type = KW_FALSE;

      } else { // else, parse the ID out of the keyword_buf, since it's clearly
               // not a keyword.
        // TODO: there has GOT to be a better way than callocing every time.
        // this sucks hard.
        unsigned long key_sz = strlen(keyword_buf);
        char *id_string_ptr =
            (char *)calloc(key_sz + 1, 1); // punn the pointer as a TokenValue,
                                           // it's 64_t so it doesn't matter.
        memcpy(id_string_ptr, keyword_buf, key_sz);
        id_string_ptr[key_sz] = '\0'; // MAKE SURE TO NULL TERM THE ID STRING
        value.as_ptr =
            id_string_ptr; // then just punn the pointer back into a
                           // TokenValue and pass it through, so the name of
                           // the ID can be accessed later in the AST.
        l_type = ID;
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

  if ((l->curr_char == '/') && (PEEK == '/')) {
    // line comment found
    RET_NEXT(2);
    while (l->curr_char != '\n') {
      RET_NEXT(1); // until the next newline, where the comment breaks.
    }
    // this skips the newline, and trims off all other whitespace.
    goto begin_next;
  }

  if ((l->curr_char == '/') && (PEEK == '*')) {
    // block comment found
    // skip both the / and *
    RET_NEXT(2);
    while ((l->curr_char != '*') && (PEEK != '/')) {
      // we haven't found the comment's end yet.
      RET_NEXT(1);
    }
    // skip past the * and /
    RET_NEXT(1);
    // if there's whitespace after the comment, we need to parse that. the
    // RET_NEXT(1); after the label will take care of the /
    goto begin_next;
  }

  l->curr_token = token_at_cursor(l);
}

void eat(Lexer *lx, Lexeme l) {
  if (lx->curr_token.type != l) {
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
  case EMPTY:
    return "EMPTY";
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

  printf("You are at X:\n");

  for (int i = 0; i < TEXT_DUMP_LEN; i++) {
    int offset_position = l->pos + i - 6;
    if (offset_position == l->pos) {
      printf("\033[41mX\033[0m"); // X with a red background
      // TODO: maybe use ansi codes to give the character a nice background?
    } else if (offset_position < l->text_len) {
      printf("%c", l->text[offset_position]);
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
