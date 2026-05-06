#include "lexer/state.hpp"

#include "lexer/alphabet.hpp"
#include "lexer/lexer.hpp"
#include "lexer/token.hpp"

namespace lexer {

StateOrToken Lexer::parse_symbol(char c, InvalidType& invalid) {
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
      invalid = InvalidType::UNEXCPECTED_SYMBOL;
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
      invalid = InvalidType::ILLEGAL_SYMBOL;
      return TokenType::INVALID;
  };
}

TokenType Lexer::parse_keyword() {
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

void Lexer::reset() {
  buffer_ = Token();
  current_ = State::START;
}

void Lexer::consume(char c) {
  buffer_.lexeme.append(1, c);
  reader_.advance();
}

bool Lexer::is_done() const {
  return reader_.eof() && current_ == State::START;
}

bool Lexer::handle_eof() {
  switch (current_) {
    case State::START:
      return false;

    case State::IN_QUOTE:
    case State::IN_CHARCON:
    case State::IN_STRING:
      buffer_.invalid_type = InvalidType::MISSING_QUOTE;
      buffer_.type = TokenType::INVALID;
      tokens_.emplace_back(buffer_);
      reset();
      return true;

    case State::IN_QUOTE_CHARCON:
    case State::END_STRING:
      buffer_.type = TokenType::STRING;
      tokens_.emplace_back(buffer_);
      reset();
      return true;

    case State::END_CHARCON:
      buffer_.type = TokenType::CHARCON;
      tokens_.emplace_back(buffer_);
      reset();
      return true;

    case State::IN_CURLY:
      buffer_.invalid_type = InvalidType::MISSING_CURLY;
      buffer_.type = TokenType::INVALID;
      tokens_.emplace_back(buffer_);
      reset();
      return true;

    case State::IN_PARENT:
      buffer_.type = TokenType::LPARENT;
      tokens_.emplace_back(buffer_);
      reset();
      return true;

    case State::IN_COMMENT_PARENT:
      buffer_.invalid_type = InvalidType::MISSING_ASTERIK;
      buffer_.type = TokenType::INVALID;
      tokens_.emplace_back(buffer_);
      reset();
      return true;

    case State::END_COMMENT_PARENT:
      buffer_.invalid_type = InvalidType::MISSING_PARENT;
      buffer_.type = TokenType::INVALID;
      tokens_.emplace_back(buffer_);
      reset();
      return true;

    case State::IN_MINUS:
      buffer_.type = TokenType::MINUS;
      tokens_.emplace_back(buffer_);
      reset();
      return true;

    case State::IN_INTCON:
      buffer_.type = TokenType::INTCON;
      tokens_.emplace_back(buffer_);
      reset();
      return true;

    case State::IN_PERIOD_INTCON:
      return handle_in_period_intcon();

    case State::IN_REALCON:
      buffer_.type = TokenType::REALCON;
      tokens_.emplace_back(buffer_);
      reset();
      return true;

    case State::IN_LESS:
      buffer_.type = TokenType::LSS;
      tokens_.emplace_back(buffer_);
      reset();
      return true;

    case State::IN_EQUAL:
      buffer_.invalid_type = InvalidType::INVALID_COMBINATION;
      buffer_.type = TokenType::INVALID;
      tokens_.emplace_back(buffer_);
      reset();
      return true;

    case State::IN_GREATER:
      buffer_.type = TokenType::GTR;
      tokens_.emplace_back(buffer_);
      reset();
      return true;

    case State::IN_COLON:
      buffer_.type = TokenType::COLON;
      tokens_.emplace_back(buffer_);
      reset();
      return true;

    case State::IN_IDENT:
      buffer_.type = parse_keyword();
      tokens_.emplace_back(buffer_);
      reset();
      return true;
  }
  return false;
}

bool Lexer::transition() {
  char c = reader_.current();
  if (is_done()) return false;  // Kalau dah selesai ya udah

  // Handle khusus EOF tapi masih ada state nggantung
  if (c == '\0') return handle_eof();

  StateOrToken next;
  InvalidType invalid = InvalidType::NOT_INVALID;

  switch (current_) {
    case State::START: {
      if (Alphabet::is_whitespace(c)) {  // Kalau whitespace abaikan
        reader_.advance();
        next = State::START;
        break;
      }

      // At this point udah bukan ws, ambil posisi buffer/lexeme
      buffer_.line_num = reader_.line_num();
      buffer_.col_num = reader_.col_num();

      if (Alphabet::is_letter(c)) {
        // Sementara kalau huruf langsung anggap ident
        consume(c);
        next = State::IN_IDENT;
      } else if (Alphabet::is_digit(c)) {
        // Kalau digit langsung menuju intcont
        consume(c);
        next = State::IN_INTCON;
      } else if (Alphabet::is_symbol(c)) {
        // Khusus parsing simbol valid
        next = parse_symbol(c, invalid);
      } else {
        // Simbol tidak dikenal, otomatis invalid (illegal symbil)
        consume(c);
        next = TokenType::INVALID;
        invalid = InvalidType::ILLEGAL_SYMBOL;
      }
      break;
    }
    case State::IN_QUOTE: {  // Kondisi ditemukan 1 (') pertama
      if (c == '\'') {
        reader_.advance();               // Bertemu 2 ' konsekutif, abaikan 1.
        next = State::IN_QUOTE_CHARCON;  // Prediksi charcon(')
      } else if (c == '\n') {            // Invalid, tidak boleh newline
        reader_.advance();               // Tidak di consume, langsung advance
        next = TokenType::INVALID;
        invalid = InvalidType::MISSING_QUOTE;
      } else {
        consume(c);
        next = State::IN_CHARCON;  // Alfabet selain ' dan \n
      }
      break;
    }
    case State::IN_QUOTE_CHARCON: {  // Ditemukan 2 konsekutif ('')
      if (c == '\'') {
        consume(c);
        next = State::IN_CHARCON;  // Ada 3 ('''), 1 pembuka, 2 dianggap (')
      } else {
        next = TokenType::STRING;  // Jangan consume, output ('') empty string
      }
      break;
    }
    case State::IN_CHARCON: {  // Kondisi 3 petik (''') atau 1 petik ('*)
      if (c == '\'') {
        consume(c);
        next = State::END_CHARCON;  // 4 konsekutif ('''') alias charcon(')
      } else {
        consume(c);
        next = State::IN_STRING;  // String yang diawalli charcon
      }
      break;
    }
    case State::END_CHARCON: {  // Kondisi 4 petik konsektif atau 3 char ('*')
      if (c == '\'') {
        reader_.advance();        // Jangan konsume, sebelumnya sudah '
        next = State::IN_STRING;  // 5 konsekutif, alias string diawali 2 petik
      } else {
        next = TokenType::CHARCON;  // Jangan consume, output charcon saat ini
      }
      break;
    }
    case State::IN_STRING: {  // Kondisi ' ganjil (1, 3, atau 5)
      if (c == '\'') {
        consume(c);
        next = State::END_STRING;  // Kondisi ' genap (menutup string)
      } else if (c == '\n') {
        reader_.advance();  // Abaikan whitespace
        invalid = InvalidType::MISSING_QUOTE;
        next = TokenType::INVALID;  // Tidak boleh newline sebelum ditutup
      } else {
        consume(c);
        next = State::IN_STRING;  // Masih string
      }
      break;
    }
    case State::END_STRING: {  // Kondisi ' genap
      if (c == '\'') {
        reader_.advance();  // 2 ' konsekutif, abaikan 1
        next = State::IN_STRING;
      } else {
        next = TokenType::STRING;  // Tutup string dan keluarkan
      }
      break;
    }
    case State::IN_CURLY: {  // { ...
      if (c == '}') {
        consume(c);
        next = TokenType::COMMENT;
      } else {
        consume(c);
        next = State::IN_CURLY;
      }
      break;
    }
    case State::IN_PARENT: {  // (
      if (c == '*') {
        consume(c);
        next = State::IN_COMMENT_PARENT;  // (*
      } else {
        next = TokenType::LPARENT;  // Simbol biasa
      }
      break;
    }
    case State::IN_COMMENT_PARENT: {
      if (c == '*') {
        consume(c);
        next = State::END_COMMENT_PARENT;  // (*...*
      } else {
        consume(c);
        next = State::IN_COMMENT_PARENT;  // Masih komen
      }
      break;
    }
    case State::END_COMMENT_PARENT: {
      if (c == ')') {
        consume(c);
        next = TokenType::COMMENT;  // (* ... *) lengkap
      } else {
        consume(c);
        next = State::IN_COMMENT_PARENT;  // Masih komen
      }
      break;
    }
    case State::IN_MINUS: {
      if (Alphabet::is_digit(c)) {
        consume(c);
        next = State::IN_INTCON;  // Bilangan negatif
      } else {
        next = TokenType::MINUS;  // Hanya simbol
      }
      break;
    }
    case State::IN_INTCON: {
      if (Alphabet::is_digit(c)) {
        consume(c);
        next = State::IN_INTCON;  // masih intcon
      } else if (c == '.') {
        consume(c);
        next = State::IN_PERIOD_INTCON;  // kondisi <digit>.
      } else {
        next = TokenType::INTCON;  // keluarkan intcon
      }
      break;
    }
    case State::IN_PERIOD_INTCON: {  // <digit>.
      if (Alphabet::is_digit(c)) {
        consume(c);
        next = State::IN_REALCON;  // masuk realcon
      } else {
        return handle_in_period_intcon();  // edge case
      }
      break;
    }
    case State::IN_REALCON: {
      if (Alphabet::is_digit(c)) {
        consume(c);
        next = State::IN_REALCON;
      } else {
        next = TokenType::REALCON;
      }
      break;
    }
    case State::IN_LESS: {
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
    }
    case State::IN_EQUAL: {
      if (c == '=') {
        consume('=');
        next = TokenType::EQL;
      } else {
        // = tidak bisa berdiri sendiri
        invalid = InvalidType::INVALID_COMBINATION;
        next = TokenType::INVALID;
      }
      break;
    }
    case State::IN_GREATER: {
      if (c == '=') {
        consume('=');
        next = TokenType::GEQ;
      } else {
        next = TokenType::GTR;
      }
      break;
    }
    case State::IN_COLON: {
      if (c == '=') {
        consume('=');
        next = TokenType::BECOMES;
      } else {
        next = TokenType::COLON;
      }
      break;
    }
    case State::IN_IDENT: {
      if (Alphabet::is_letter(c) || Alphabet::is_digit(c)) {
        consume(c);
        next = State::IN_IDENT;
      } else {
        next = parse_keyword();
      }
      break;
    }
    default: {
      // default error
      invalid = InvalidType::UNEXCPECTED_SYMBOL;
      next = TokenType::INVALID;
      break;
    }
  }

  if (std::holds_alternative<State>(next)) {
    current_ = std::get<State>(next);  // transisi ke state selanjutnya
    return false;
  } else if (std::holds_alternative<TokenType>(next)) {
    TokenType type = std::get<TokenType>(next);
    buffer_.type = type;
    buffer_.invalid_type =
        (type == TokenType::INVALID) ? invalid : InvalidType::NOT_INVALID;
    tokens_.emplace_back(buffer_);  // emit token
    reset();
    return true;
  }

  return false;
}

bool Lexer::handle_in_period_intcon() {
  // emit intcon dengan menghapus . di akhir
  buffer_.lexeme.pop_back();
  buffer_.type = TokenType::INTCON;
  tokens_.emplace_back(buffer_);

  // logic untuk mendapatkan lokasi .
  int line = reader_.line_num();
  int col = reader_.col_num();
  if (col == 1) {
    col = buffer_.col_num + buffer_.lexeme.length();
    line--;
  }

  // emit . dan reset lembali
  tokens_.emplace_back(
      Token{TokenType::PERIOD, InvalidType::NOT_INVALID, "", line, col});

  reset();
  return true;
}

}  // namespace lexer