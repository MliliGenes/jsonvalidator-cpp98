#include "parser.hpp"
#include <cctype>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

token extract_string(std::istream &in) {
  token tk;
  char c = 'c';

  while (1) {
    in.get(c);
    if (c == '"')
      break;
    if (in.eof())
      return tk.type = TK_UNDEFINED, tk;
    if (c == '\\') {
      if (!in.get(c))
        break;

      switch (c) {
      case '"':
        tk.token.push_back('"');
        break;
      case '\\':
        tk.token.push_back('\\');
        break;
      case '/':
        tk.token.push_back('/');
        break;
      case 'b':
        tk.token.push_back('\b');
        break;
      case 'f':
        tk.token.push_back('\f');
        break;
      case 'n':
        tk.token.push_back('\n');
        break;
      case 'r':
        tk.token.push_back('\r');
        break;
      case 't':
        tk.token.push_back('\t');
        break;
      default:
        tk.token.push_back(c);
        break;
      }
    } else
      tk.token.push_back(c);
  }
  tk.type = TK_STRING;
  return tk;
}

token extract_decimal(char &c, std::istream &in) {
  token tk;
  int dots = 0;

  if (c == '-') {
    tk.token.push_back(c);
    in.get(c);
  }

  while (1) {
    if (in.eof() || dots > 1)
      return tk.type = TK_UNDEFINED, tk;
    if (c == '.')
      dots++;
    else if (!isdigit(c)) {
      in.unget();
      break;
    }
    tk.token.push_back(c);
    in.get(c);
  }
  if (tk.token[tk.token.length() - 1] == '.')
    return tk.type = TK_UNDEFINED, tk;
  tk.type = !dots ? TK_NUMBER : TK_DOUBLE;
  return tk;
}

token extract_json_types(char &c, std::istream &in) {
  token tk;
  tk.token.push_back(c);
  int i = 1;
  int len = (c == 'f' ? 5 : 4);
  while (i < len) {
    if (in.eof())
      break;
    in.get(c);
    tk.token.push_back(c);
    i++;
  }
  if (tk.token == "true" || tk.token == "false")
    tk.type = TK_BOOLEAN;
  else if (tk.token == "null")
    tk.type = TK_NIL;
  else
    tk.type = TK_UNDEFINED;
  return tk;
}

token Tokenizer::extract_token(std::istream &in) {
  token tk;
  char c = 'c';

  tk.type = TK_UNDEFINED;
  while (!in.eof()) {
    in.get(c);
    if (!isspace(c))
      break;
  }
  if (!c || in.eof())
    tk.type = END;
  else if (c == '{')
    tk.type = CB_OPEN;
  else if (c == '}')
    tk.type = CB_CLOSE;
  else if (c == '[')
    tk.type = SB_OPEN;
  else if (c == ']')
    tk.type = SB_CLOSE;
  else if (c == ':')
    tk.type = COLON;
  else if (c == ',')
    tk.type = COMMA;
  else if (c == '"')
    tk = extract_string(in);
  else if (isdigit(c) || c == '-')
    tk = extract_decimal(c, in);
  else if (c == 'n' || c == 't' || c == 'f')
    tk = extract_json_types(c, in);
  return tk;
}

void Tokenizer::parse(std::istream &in, std::vector<token> &tokens) {

  while (1) {
    token tk = extract_token(in);
    tokens.push_back(tk);
    if (tk.type == END)
      break;
  }
}

std::string match_token_name(token_type type) {
  if (type == CB_OPEN)
    return "CB_OPEN";
  else if (type == CB_CLOSE)
    return "CB_CLOSE";
  else if (type == SB_OPEN)
    return "SB_OPEN";
  else if (type == SB_CLOSE)
    return "SB_CLOSE";
  else if (type == TK_STRING)
    return "STRING";
  else if (type == TK_DOUBLE)
    return "DOUBLE";
  else if (type == TK_NUMBER)
    return "NUMBER";
  else if (type == COMMA)
    return "COMMA";
  else if (type == COLON)
    return "COLON";
  else if (type == TK_BOOLEAN)
    return "BOOLEAN";
  else if (type == TK_NIL)
    return "NULL";
  else if (type == END)
    return "END";
  return "TK_UNDEFINED";
}

std::ostream &operator<<(std::ostream &os, token &tk) {
  if (tk.type == TK_STRING)
    os << match_token_name(tk.type) << "(" << tk.token << ")";
  else if (tk.type == TK_NUMBER)
    os << match_token_name(tk.type) << "(" << tk.token << ")";
  else if (tk.type == TK_DOUBLE)
    os << match_token_name(tk.type) << "(" << tk.token << ")";
  else if (tk.type == TK_BOOLEAN || tk.type == TK_NIL)
    os << match_token_name(tk.type) << "(" << tk.token << ")";
  else
    os << match_token_name(tk.type);
  return os;
}
bool lexer_helper(std::vector<token> &tokens, unsigned long &pos);
bool object_lexer(std::vector<token> &tokens, unsigned long &pos);
bool array_lexer(std::vector<token> &tokens, unsigned long &pos);

bool lexer_consume(std::vector<token> &tokens, unsigned long &pos) {
  token_type tk1 = tokens[pos + 1].type;
  token_type tk2 = tokens[pos + 2].type;
  if (pos + 3 > tokens.size() || tk1 != COLON)
    return 1;
  if (tk2 == CB_OPEN || tk2 == SB_OPEN) {
    pos += 2;
    if (lexer_helper(tokens, pos))
      return 1;
  } else if (tk2 == TK_STRING || tk2 == TK_BOOLEAN || tk2 == TK_NIL ||
             tk2 == TK_NUMBER || tk2 == TK_DOUBLE)
    pos += 3;
  else
    return 1;
  return 0;
}

bool object_lexer(std::vector<token> &tokens, unsigned long &pos) {
  pos++;
  while (pos < tokens.size() && tokens[pos].type != CB_CLOSE) {
    if (tokens[pos].type != TK_STRING)
      return 1;
    if (lexer_consume(tokens, pos))
      return 1;
    if (tokens[pos].type == COMMA && tokens[pos + 1].type != TK_STRING)
      return 1;
    if (tokens[pos].type == COMMA)
      pos++;
    else
      break;
  }
  if (tokens[pos].type == CB_CLOSE)
    pos++;
  else
    return 1;
  return 0;
}

static bool is_array_element(token_type type) {
  return type == TK_STRING || type == SB_OPEN || type == TK_NUMBER ||
         type == TK_DOUBLE || type == CB_OPEN || type == TK_NIL ||
         type == TK_BOOLEAN;
}

bool array_lexer(std::vector<token> &tokens, unsigned long &pos) {
  pos++;
  while (pos < tokens.size() && tokens[pos].type != SB_CLOSE) {
    if (tokens[pos].type == CB_OPEN && object_lexer(tokens, pos))
      return 1;
    else if (tokens[pos].type == SB_OPEN && array_lexer(tokens, pos))
      return 1;
    else if (is_array_element(tokens[pos].type))
      pos++;
    if (tokens[pos].type == COMMA && !is_array_element(tokens[pos + 1].type))
      return 1;
    if (tokens[pos].type == COMMA)
      pos++;
    else
      break;
  }
  if (tokens[pos].type == SB_CLOSE)
    pos++;
  else
    return 1;
  return 0;
}

bool lexer_helper(std::vector<token> &tokens, unsigned long &pos) {
  if (tokens[pos].type == CB_OPEN) {
    if (object_lexer(tokens, pos))
      return 1;
  } else if (tokens[pos].type == SB_OPEN) {
    if (array_lexer(tokens, pos))
      return 1;
  } else {
    return 1;
  }
  return 0;
}

std::vector<token> &Lexer::parse(std::vector<token> &tokens) {
  unsigned long pos = 0;
  while (pos < tokens.size() && tokens[pos].type != END) {
    if (lexer_helper(tokens, pos)) {
      throw std::runtime_error("Malformed JSON file!");
    }
  }
  return tokens;
}
