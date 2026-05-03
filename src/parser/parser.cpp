
#include "parser/parser.hpp"

#include <vector>

#include "arion_exceptions.hpp"
#include "lexer/token.hpp"

namespace parser {

Parser::Parser(const std::string& filename,
               const std::vector<lexer::Token>& tokens)
    : filename_(filename),
      tokens_(tokens),
      position_(0),
      program_(NodeType::Program) {}

void Parser::parse() { parseProgram(); }

bool Parser::is_done() const { return position_ == tokens_.size(); }

const lexer::Token& Parser::current() const {
  if (position_ >= tokens_.size()) {
    throw EOFTokenLookupException(filename_);
  }
  return tokens_.at(position_);
}

const lexer::Token& Parser::peek(int ahead) const {
  if (position_ + ahead >= tokens_.size()) {
    throw EOFTokenLookupException(filename_);
  }
  return tokens_.at(position_ + ahead);
}

ParsePtr Parser::consume(lexer::TokenType expected) {
  if (position_ >= tokens_.size()) {
    throw EOFTokenLookupException(filename_);
  }

  if (current().type != expected) {
    throw UnexpectedTokenException(filename_, expected, current());
  }

  return std::make_unique<ParseNode>(NodeType::TokenNode,
                                     tokens_.at(position_++));
}

}  // namespace parser