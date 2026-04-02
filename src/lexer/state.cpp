#include "state.hpp"

#include "alphabet.hpp"
#include "lexer.hpp"

StateOrToken Lexer::parse_symbol(char c) {
  switch (c) {
    case '+':
      consume('+');
      return TokenType::PLUS;
    case '-':
      consume('-');
      return State::IN_MINUS;
    case '*':
      consume('*');
      return TokenType::TIMES;
    case '/':
      consume('/');
      return TokenType::RDIV;
    case '=':
      consume('=');
      return State::IN_EQUAL;
    case '<':
      consume('<');
      return State::IN_LESS;
    case '>':
      consume('>');
      return State::IN_GREATER;
    case '(':
      consume('(');
      return State::IN_PARENT;
    case ')':
      consume(')');
      return TokenType::RPARENT;
    case '[':
      consume('[');
      return TokenType::LBRACK;
    case ']':
      consume(']');
      return TokenType::RBRACK;
    case '{':
      consume('{');
      return State::IN_CURLY;
    case '}':
      consume('}');
      return TokenType::INVALID;
    case '.':
      consume('.');
      return TokenType::PERIOD;
    case ',':
      consume(',');
      return TokenType::COMMA;
    case ';':
      consume(';');
      return TokenType::SEMICOLON;
    case ':':
      consume(':');
      return State::IN_COLON;
    case '\'':
      consume('\'');
      return State::IN_QUOTE;
    default:
      consume(c);
      return TokenType::INVALID;
  };
}

StateOrToken Lexer::parse_keyword() {
  std::string buf = Alphabet::to_lower(buffer_.lexeme);
  if (buf == "and") return TokenType::ANDSY;
  if (buf == "array") return TokenType::ARRAYSY;
  if (buf == "begin") return TokenType::BEGINSY;
  if (buf == "case") return TokenType::CASESY;
  if (buf == "const") return TokenType::CONSTSY;
  if (buf == "div") return TokenType::IDIV;
  if (buf == "do") return TokenType::DOSY;
  if (buf == "downto") return TokenType::DOWNTOSY;
  if (buf == "else") return TokenType::ELSESY;
  if (buf == "end") return TokenType::ENDSY;
  if (buf == "for") return TokenType::FORSY;
  if (buf == "function") return TokenType::FUNCTIONSY;
  if (buf == "if") return TokenType::IFSY;
  if (buf == "mod") return TokenType::IMOD;
  if (buf == "not") return TokenType::NOTSY;
  if (buf == "of") return TokenType::OFSY;
  if (buf == "or") return TokenType::ORSY;
  if (buf == "procedure") return TokenType::PROCEDURESY;
  if (buf == "program") return TokenType::PROGRAMSY;
  if (buf == "record") return TokenType::RECORDSY;
  if (buf == "repeat") return TokenType::REPEATSY;
  if (buf == "then") return TokenType::THENSY;
  if (buf == "to") return TokenType::TOSY;
  if (buf == "type") return TokenType::TYPESY;
  if (buf == "until") return TokenType::UNTILSY;
  if (buf == "var") return TokenType::VARSY;
  if (buf == "while") return TokenType::WHILESY;
  return TokenType::IDENT;
}

void Lexer::consume(char c) {
  buffer_.lexeme.append(1, c);
  reader_.advance();
}

bool Lexer::is_done() const {
  return reader_.current() == '\0' && current_ == State::START;
}

bool Lexer::transition() {
  char c = reader_.current();
  if (c == '\0' && current_ == State::START) return false;

  StateOrToken next;

  switch (current_) {
    case State::START: {
      if (Alphabet::is_whitespace(c)) {
        reader_.advance();
        next = State::START;
        break;
      }

      buffer_.line_num = reader_.line_num();
      buffer_.col_num = reader_.col_num();

      if (Alphabet::is_letter(c)) {
        consume(c);
        next = State::IN_IDENT;
      } else if (Alphabet::is_digit(c)) {
        consume(c);
        next = State::IN_INTCON;
      } else if (Alphabet::is_symbol(c)) {
        next = parse_symbol(c);
      } else {
        consume(c);
        next = TokenType::INVALID;
      }
      break;
    }
    case State::IN_QUOTE:
      if (c == '\'') {
        consume('\'');
        next = State::IN_QUOTE_CHARCON;
      } else if (c == '\n') {
        consume('\n');
        next = TokenType::INVALID;
      } else {
        consume(c);
        next = State::IN_CHARCON;
      }
      break;
    case State::IN_QUOTE_CHARCON:
      if (c == '\'') {
        consume('\'');
        next = State::IN_CHARCON;
      } else {
        consume(c);
        next = TokenType::INVALID;
      }
      break;
    case State::IN_CHARCON:
      if (c == '\'') {
        consume('\'');
        next = State::END_CHARCON;
      } else {
        consume(c);
        next = State::IN_STRING;
      }
      break;
    case State::END_CHARCON:
      if (c == '\'') {
        consume('\'');
        next = State::IN_CHARCON;
      } else {
        consume(c);
        next = TokenType::INVALID;
      }
      break;
    case State::IN_STRING:
      if (c == '\'') {
        consume('\'');
        next = State::END_STRING;
      } else if (c == '\n') {
        consume('\n');
        next = TokenType::INVALID;
      } else {
        consume(c);
        next = State::IN_STRING;
      }
      break;
    case State::END_STRING:
      if (c == '\'') {
        consume('\'');
        next = State::IN_STRING;
      } else {
        next = TokenType::STRING;
      }
      break;
    case State::IN_CURLY:
      if (c == '}') {
        consume('}');
        next = TokenType::COMMENT;
      } else {
        consume(c);
        next = State::IN_CURLY;
      }
      break;
    case State::IN_PARENT:
      if (c == '*') {
        consume('*');
        next = State::IN_COMMENT_PARENT;
      } else {
        next = TokenType::LPARENT;
      }
      break;
    case State::IN_COMMENT_PARENT:
      if (c == '*') {
        consume('*');
        next = State::END_COMMENT_PARENT;
      } else {
        consume(c);
        next = State::IN_COMMENT_PARENT;
      }
      break;
    case State::END_COMMENT_PARENT:
      if (c == ')') {
        consume(')');
        next = TokenType::COMMENT;
      } else {
        consume(c);
        next = State::IN_COMMENT_PARENT;
      }
      break;
    case State::IN_MINUS:
      if (Alphabet::is_digit(c)) {
        consume(c);
        next = State::IN_INTCON;
      } else {
        next = TokenType::MINUS;
      }
      break;
    case State::IN_INTCON:
      if (Alphabet::is_digit(c)) {
        consume(c);
        next = State::IN_INTCON;
      } else if (c == '.') {
        consume('.');
        next = State::IN_PERIOD_INTCON;
      } else {
        next = TokenType::INTCON;
      }
      break;
    case State::IN_PERIOD_INTCON:
      if (Alphabet::is_digit(c)) {
        consume(c);
        next = State::IN_REALCON;
      } else {
        next = TokenType::INTCON;
      }
      break;
    case State::IN_REALCON:
      if (Alphabet::is_digit(c)) {
        consume(c);
        next = State::IN_REALCON;
      } else {
        next = TokenType::REALCON;
      }
      break;
    case State::IN_LESS:
      if (c == '=') {
        consume('=');
        next = TokenType::LEQ;
      } else if (c == '>') {
        consume('>');
        next = TokenType::NEQ;
      } else {
        next = TokenType::LSS;
      }
      break;
    case State::IN_EQUAL:
      if (c == '=') {
        consume('=');
        next = TokenType::EQL;
      } else {
        next = TokenType::INVALID;
      }
      break;
    case State::IN_GREATER:
      if (c == '=') {
        consume('=');
        next = TokenType::GEQ;
      } else {
        next = TokenType::GTR;
      }
      break;
    case State::IN_COLON:
      if (c == '=') {
        consume('=');
        next = TokenType::BECOMES;
      } else {
        next = TokenType::COLON;
      }
      break;
    case State::IN_IDENT:
      if (Alphabet::is_letter(c) || Alphabet::is_digit(c)) {
        consume(c);
        next = State::IN_IDENT;
      } else {
        next = parse_keyword();
      }
      break;
    default:
      next = TokenType::INVALID;
      break;
  }

  if (std::holds_alternative<State>(next)) {
    current_ = std::get<State>(next);
    return false;
  } else if (std::holds_alternative<TokenType>(next)) {
    TokenType type = std::get<TokenType>(next);
    buffer_.type = type;
    tokens_.push_back(buffer_);
    buffer_ = Token();
    current_ = State::START;
    return true;
  }

  return false;
}
