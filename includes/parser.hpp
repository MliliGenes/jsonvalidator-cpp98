#pragma once

#include <string>
#include <vector>

typedef enum token_type {
  CB_OPEN,
  CB_CLOSE,
  SB_OPEN,
  SB_CLOSE,
  TK_STRING,
  TK_NUMBER,
  TK_DOUBLE,
  COMMA,
  COLON,
  END,
  TK_BOOLEAN,
  TK_NIL,
  TK_UNDEFINED
} t_type;

typedef struct token {
  std::string token;
  t_type type;
} token;

class Tokenizer {
public:
  static token extract_token(std::istream &);
  static void parse(std::istream &, std::vector<token> &);
};

class Lexer {
public:
  static std::vector<token> &parse(std::vector<token> &);
};

std::ostream &operator<<(std::ostream &os, token &tk);
