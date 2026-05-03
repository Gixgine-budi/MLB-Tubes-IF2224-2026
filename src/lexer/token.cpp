#include "token.hpp"

#include <ostream>

namespace lexer {

std::ostream& operator<<(std::ostream& os, const Token& t) {
  switch (t.type) {
    case TokenType::INVALID:
      os << "unknown(" << t.lexeme << ")";
      break;
    case TokenType::INTCON:
      os << "intcon(" << t.lexeme << ")";
      break;
    case TokenType::REALCON:
      os << "realcon(" << t.lexeme << ")";
      break;
    case TokenType::CHARCON:
      os << "charcon(" << t.lexeme << ")";
      break;
    case TokenType::STRING:
      os << "string(" << t.lexeme << ")";
      break;
    case TokenType::NOTSY:
      os << "notsy";
      break;
    case TokenType::PLUS:
      os << "plus";
      break;
    case TokenType::MINUS:
      os << "minus";
      break;
    case TokenType::TIMES:
      os << "times";
      break;
    case TokenType::IDIV:
      os << "idiv";
      break;
    case TokenType::RDIV:
      os << "rdiv";
      break;
    case TokenType::IMOD:
      os << "imod";
      break;
    case TokenType::ANDSY:
      os << "andsy";
      break;
    case TokenType::ORSY:
      os << "orsy";
      break;
    case TokenType::EQL:
      os << "eql";
      break;
    case TokenType::NEQ:
      os << "neq";
      break;
    case TokenType::GTR:
      os << "gtr";
      break;
    case TokenType::GEQ:
      os << "geq";
      break;
    case TokenType::LSS:
      os << "lss";
      break;
    case TokenType::LEQ:
      os << "leq";
      break;
    case TokenType::LPARENT:
      os << "lparent";
      break;
    case TokenType::RPARENT:
      os << "rparent";
      break;
    case TokenType::LBRACK:
      os << "lbrack";
      break;
    case TokenType::RBRACK:
      os << "rbrack";
      break;
    case TokenType::COMMA:
      os << "comma";
      break;
    case TokenType::SEMICOLON:
      os << "semicolon";
      break;
    case TokenType::PERIOD:
      os << "period";
      break;
    case TokenType::COLON:
      os << "colon";
      break;
    case TokenType::BECOMES:
      os << "becomes";
      break;
    case TokenType::CONSTSY:
      os << "constsy";
      break;
    case TokenType::TYPESY:
      os << "typesy";
      break;
    case TokenType::VARSY:
      os << "varsy";
      break;
    case TokenType::FUNCTIONSY:
      os << "functionsy";
      break;
    case TokenType::PROCEDURESY:
      os << "proceduresy";
      break;
    case TokenType::ARRAYSY:
      os << "arraysy";
      break;
    case TokenType::RECORDSY:
      os << "recordsy";
      break;
    case TokenType::PROGRAMSY:
      os << "programsy";
      break;
    case TokenType::IDENT:
      os << "ident(" << t.lexeme << ")";
      break;
    case TokenType::BEGINSY:
      os << "beginsy";
      break;
    case TokenType::IFSY:
      os << "ifsy";
      break;
    case TokenType::CASESY:
      os << "casesy";
      break;
    case TokenType::REPEATSY:
      os << "repeatsy";
      break;
    case TokenType::WHILESY:
      os << "whilesy";
      break;
    case TokenType::FORSY:
      os << "forsy";
      break;
    case TokenType::ENDSY:
      os << "endsy";
      break;
    case TokenType::ELSESY:
      os << "elsesy";
      break;
    case TokenType::UNTILSY:
      os << "untilsy";
      break;
    case TokenType::OFSY:
      os << "ofsy";
      break;
    case TokenType::DOSY:
      os << "dosy";
      break;
    case TokenType::TOSY:
      os << "tosy";
      break;
    case TokenType::DOWNTOSY:
      os << "downtosy";
      break;
    case TokenType::THENSY:
      os << "thensy";
      break;
    case TokenType::COMMENT:
      os << "comment(" << t.lexeme << ")";
      break;
  }
  return os;
}

const std::string Token::error_message() const {
  switch (invalid_type) {
    case InvalidType::NOT_INVALID:
      return "";
    case InvalidType::ILLEGAL_SYMBOL:
      return std::string("found illegal symbol \'").append(lexeme).append("\'");
    case InvalidType::MISSING_QUOTE:
      return std::string("missing closing quote \' at\n")
          .append(std::to_string(line_num))
          .append("\t | ... ")
          .append(lexeme)
          .append(" ...");
    case InvalidType::MISSING_CURLY:
      return std::string("missing closing } for comment at\n")
          .append(std::to_string(line_num))
          .append("\t | ... ")
          .append(lexeme)
          .append(" ...");
    case InvalidType::MISSING_PARENT:
      return std::string(" incomplete closing, missing ) for comment at\n")
          .append(std::to_string(line_num))
          .append("\t | ... ")
          .append(lexeme)
          .append(" ...");
    case InvalidType::MISSING_ASTERIK:
      return std::string("missing closing *) for comment for at\n")
          .append(std::to_string(line_num))
          .append("\t | ... ")
          .append(lexeme)
          .append(" ...");
    case InvalidType::INVALID_COMBINATION:
      return std::string("invalid symbol combination \'")
          .append(lexeme)
          .append("\'");
    case InvalidType::UNEXCPECTED_SYMBOL:
      return std::string("found unexpected symbol \'")
          .append(lexeme)
          .append("\'");
  }
  return "";
}

}  // namespace lexer