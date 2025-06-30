#include "Json.hpp"
#include "AJsonValue.hpp"
#include "JsonTypes.hpp"
#include "parser.hpp"
#include <fstream>
#include <sstream>
#include <vector>

AJsonValue *match_type(token &value) {
  if (value.type == TK_STRING)
    return new JsonString(value.token);
  else if (value.type == TK_NUMBER)
    return new JsonNumber(value.token);
  else if (value.type == TK_DOUBLE)
    return new JsonDouble(value.token);
  else if (value.type == TK_BOOLEAN)
    return new JsonBool(value.token);
  return new JsonNull();
}
AJsonValue *parse_helper(std::vector<token> &tokens, unsigned long &pos);
void consume_key(std::vector<token> &tokens, unsigned long &pos,
                 JsonObject &object) {
  token key = tokens[pos];
  pos += 2;
  token value = tokens[pos];
  if (object.members.find(key.token) != object.members.end())
    delete object.members[key.token];
  if (value.type == CB_OPEN || value.type == SB_OPEN) {
    object.members[key.token] = parse_helper(tokens, pos);
  } else {
    object.members[key.token] = match_type(value);
    pos++;
  }
}

AJsonValue *parse_object(std::vector<token> &tokens, unsigned long &pos) {
  JsonObject *object = new JsonObject();
  pos++;
  while (pos < tokens.size() && tokens[pos].type != CB_CLOSE) {
    consume_key(tokens, pos, *object);
    if (tokens[pos].type == COMMA)
      pos++;
  }
  pos++;
  return object;
}

static bool is_array_element(token_type type) {
  return type == TK_STRING || type == TK_NUMBER || type == TK_DOUBLE ||
         type == TK_NIL || type == TK_BOOLEAN;
}

AJsonValue *parse_array(std::vector<token> &tokens, unsigned long &pos) {
  JsonArray *array = new JsonArray();
  pos++;
  while (pos < tokens.size() && tokens[pos].type != SB_CLOSE) {
    if (tokens[pos].type == CB_OPEN)
      array->elements.push_back(parse_object(tokens, pos));
    else if (tokens[pos].type == SB_OPEN)
      array->elements.push_back(parse_array(tokens, pos));
    else if (is_array_element(tokens[pos].type)) {
      array->elements.push_back(match_type(tokens[pos]));
      pos++;
    }
    if (tokens[pos].type == COMMA)
      pos++;
  }
  if (tokens[pos].type == SB_CLOSE)
    pos++;
  return array;
}

AJsonValue *parse_helper(std::vector<token> &tokens, unsigned long &pos) {
  AJsonValue *json = NULL;

  if (tokens[pos].type == CB_OPEN)
    json = parse_object(tokens, pos);
  else if (tokens[pos].type == SB_OPEN)
    json = parse_array(tokens, pos);
  return json;
}

AJsonValue *Json::parse(std::string filename) {
  std::ifstream in(filename.c_str());
  if (!in.is_open()) {
    throw std::runtime_error("Could not open file: " + filename);
  }
  std::vector<token> tokens;
  Tokenizer::parse(in, tokens);
  in.close();
  Lexer::parse(tokens);
  unsigned long pos = 0;
  return parse_helper(tokens, pos);
}

AJsonValue *Json::parse_raw(std::string raw) {
  std::istringstream is(raw.c_str());
  std::vector<token> tokens;
  Tokenizer::parse(is, tokens);
  Lexer::parse(tokens);
  unsigned long pos = 0;
  return parse_helper(tokens, pos);
}
