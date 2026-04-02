#include "state.hpp"

#include "alphabet.hpp"
#include "lexer.hpp"
#include "token.hpp"

StateOrToken Lexer::State::parse_symbol(char c) {
  switch (c) {
    case '+':
      consume('+');
      return TokenType::PLUS;
    case '-':
      consume('-');
      return StateName::IN_MINUS;
    case '*':
      consume('*');
      return TokenType::TIMES;
    case '/':
      consume('/');
      return TokenType::RDIV;
    case '=':
      consume('=');
      return StateName::IN_EQUAL;
    case '<':
      consume('<');
      return StateName::IN_LESS;
    case '>':
      consume('>');
      return StateName::IN_GREATER;
    case '(':
      consume('(');
      return StateName::IN_PARENT;
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
      return StateName::IN_CURLY;
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
      return StateName::IN_COLON;
    case '\'':
      consume('\'');
      return StateName::IN_QUOTE;
    default:
      consume(c);
      return TokenType::INVALID;
  };
}

StateOrToken Lexer::State::parse_keyword() {
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

Lexer::State::State(StateName name, Lexer& lexer) : name_(name), lexer_(lexer) {
  auto start = [this](char c) -> StateOrToken {
    if (Alphabet::is_whitespace(c)) {
      lexer_.reader_.advance();
      return name_;
    }

    buffer_.line_num = lexer_.reader_.line_num();
    buffer_.col_num = lexer_.reader_.col_num();

    if (Alphabet::is_letter(c)) {
      consume(c);
      return StateName::IN_IDENT;
    }

    if (Alphabet::is_digit(c)) {
      consume(c);
      return StateName::IN_INTCON;
    }

    if (Alphabet::is_symbol(c)) return parse_symbol(c);

    consume(c);
    return TokenType::INVALID;
  };

  auto in_quote = [this](char c) -> StateOrToken {
    if (c == '\'') {
      consume('\'');
      return StateName::IN_QUOTE_CHARCON;
    } else if (c == '\n') {
      consume('\n');
      return TokenType::INVALID;
    } else {
      consume(c);
      return StateName::IN_CHARCON;
    }
  };

  auto in_quote_charcon = [this](char c) -> StateOrToken {
    if (c == '\'') {
      consume('\'');
      return StateName::IN_CHARCON;
    } else {
      consume(c);
      return TokenType::INVALID;
    }
  };

  auto in_charcon = [this](char c) -> StateOrToken {
    if (c == '\'') {
      consume('\'');
      return StateName::END_CHARCON;
    } else {
      consume(c);
      return StateName::IN_STRING;
    }
  };

  auto end_charcon = [this](char c) -> StateOrToken {
    if (c == '\'') {
      consume('\'');
      return StateName::IN_CHARCON;
    } else {
      consume(c);
      return TokenType::INVALID;
    }
  };

  auto in_string = [this](char c) -> StateOrToken {
    if (c == '\'') {
      consume('\'');
      return StateName::END_STRING;
    } else if (c == '\n') {
      consume('\n');
      return TokenType::INVALID;
    } else {
      consume(c);
      return name_;
    }
  };

  auto end_string = [this](char c) -> StateOrToken {
    if (c == '\'') {
      consume('\'');
      return StateName::IN_STRING;
    } else {
      return TokenType::STRING;
    }
  };

  auto in_curly = [this](char c) -> StateOrToken {
    if (c == '}') {
      consume('}');
      return TokenType::COMMENT;
    } else {
      consume(c);
      return name_;
    }
  };

  auto in_parent = [this](char c) -> StateOrToken {
    if (c == '*') {
      consume('*');
      return StateName::IN_COMMENT_PARENT;
    } else {
      return TokenType::LPARENT;
    }
  };

  auto in_comment_parent = [this](char c) -> StateOrToken {
    if (c == '*') {
      consume('*');
      return StateName::END_COMMENT_PARENT;
    } else {
      consume(c);
      return name_;
    }
  };

  auto end_comment_parent = [this](char c) -> StateOrToken {
    if (c == ')') {
      consume(')');
      return TokenType::COMMENT;
    } else {
      consume(c);
      return name_;
    }
  };

  auto in_minus = [this](char c) -> StateOrToken {
    if (Alphabet::is_digit(c)) {
      consume(c);
      return StateName::IN_INTCON;
    } else {
      return TokenType::MINUS;
    }
  };

  auto in_intcon = [this](char c) -> StateOrToken {
    if (Alphabet::is_digit(c)) {
      consume(c);
      return name_;
    } else if (c == '.') {
      consume('.');
      return StateName::IN_PERIOD_INTCON;
    } else {
      return TokenType::INTCON;
    }
  };

  auto in_period_intcon = [this](char c) -> StateOrToken {
    if (Alphabet::is_digit(c)) {
      consume(c);
      return StateName::IN_REALCON;
    } else {
      // TODO: ALSO RETURN THE PERIOD AS A SEPARATE TOKEN
      return TokenType::INTCON;
    }
  };

  auto in_realcon = [this](char c) -> StateOrToken {
    if (Alphabet::is_digit(c)) {
      consume(c);
      return name_;
    } else {
      return TokenType::REALCON;
    }
  };

  auto in_less = [this](char c) -> StateOrToken {
    if (c == '=') {
      consume('=');
      return TokenType::LEQ;
    } else if (c == '>') {
      consume('>');
      return TokenType::NEQ;
    } else {
      return TokenType::LSS;
    }
  };

  auto in_equal = [this](char c) -> StateOrToken {
    if (c == '=') {
      consume('=');
      return TokenType::EQL;
    } else {
      return TokenType::INVALID;
    }
  };

  auto in_greater = [this](char c) -> StateOrToken {
    if (c == '=') {
      consume('=');
      return TokenType::GEQ;
    } else {
      return TokenType::GTR;
    }
  };

  auto in_colon = [this](char c) -> StateOrToken {
    if (c == '=') {
      consume('=');
      return TokenType::BECOMES;
    } else {
      return TokenType::COLON;
    }
  };

  auto in_ident = [this](char c) -> StateOrToken {
    if (Alphabet::is_letter(c) || Alphabet::is_digit(c)) {
      consume(c);
      return name_;
    } else {
      return parse_keyword();
    }
  };

  lookup_[static_cast<int>(StateName::START)] = start;
  lookup_[static_cast<int>(StateName::IN_QUOTE)] = in_quote;
  lookup_[static_cast<int>(StateName::IN_QUOTE_CHARCON)] = in_quote_charcon;
  lookup_[static_cast<int>(StateName::IN_CHARCON)] = in_charcon;
  lookup_[static_cast<int>(StateName::END_CHARCON)] = end_charcon;
  lookup_[static_cast<int>(StateName::IN_STRING)] = in_string;
  lookup_[static_cast<int>(StateName::END_STRING)] = end_string;
  lookup_[static_cast<int>(StateName::IN_CURLY)] = in_curly;
  lookup_[static_cast<int>(StateName::IN_PARENT)] = in_parent;
  lookup_[static_cast<int>(StateName::IN_COMMENT_PARENT)] = in_comment_parent;
  lookup_[static_cast<int>(StateName::END_COMMENT_PARENT)] = end_comment_parent;
  lookup_[static_cast<int>(StateName::IN_MINUS)] = in_minus;
  lookup_[static_cast<int>(StateName::IN_INTCON)] = in_intcon;
  lookup_[static_cast<int>(StateName::IN_PERIOD_INTCON)] = in_period_intcon;
  lookup_[static_cast<int>(StateName::IN_REALCON)] = in_realcon;
  lookup_[static_cast<int>(StateName::IN_LESS)] = in_less;
  lookup_[static_cast<int>(StateName::IN_EQUAL)] = in_equal;
  lookup_[static_cast<int>(StateName::IN_GREATER)] = in_greater;
  lookup_[static_cast<int>(StateName::IN_COLON)] = in_colon;
  lookup_[static_cast<int>(StateName::IN_IDENT)] = in_ident;
}

void Lexer::State::consume(char c) {
  buffer_.lexeme.append(1, c);
  lexer_.reader_.advance();
}

bool Lexer::State::is_done() const {
  return lexer_.reader_.current() == '\0' && name_ == StateName::START;
}

bool Lexer::State::transition() {
  char c = lexer_.reader_.current();
  if (c == '\0' && name_ == StateName::START) return false;

  StateOrToken next = lookup_[static_cast<int>(name_)](c);

  if (std::holds_alternative<StateName>(next)) {
    name_ = std::get<StateName>(next);
    return false;
  } else if (std::holds_alternative<TokenType>(next)) {
    TokenType type = std::get<TokenType>(next);
    buffer_.type = type;
    lexer_.tokens_.push_back(buffer_);
    buffer_ = Token();
    name_ = StateName::START;
    return true;
  } else
    return false;
}