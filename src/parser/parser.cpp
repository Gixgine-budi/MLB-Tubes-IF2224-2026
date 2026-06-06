#include "parser/parser.hpp"

#include <initializer_list>
#include <string>
#include <vector>

#include "diagnoser/diagnostic.hpp"
#include "lexer/token.hpp"
#include "parser/parse_node.hpp"

namespace parser {

static const lexer::Token eof_sentinel{};

Parser::Parser(const std::string& filename,
               const std::vector<lexer::Token>& tokens,
               diag::Diagnoser& diagnoser)
    : filename_(filename),
      tokens_(tokens),
      position_(0),
      program_(NodeType::Program),
      diagnoser_(diagnoser) {}

void Parser::parse() { parseProgram(); }

bool Parser::is_done() const { return position_ >= tokens_.size(); }

const lexer::Token& Parser::current() const {
  if (position_ >= tokens_.size()) {
    if (!eof_reported_) {
      diagnoser_.report({diag::Phase::PARSER,
                         diag::Level::ERROR,
                         {0, 1, 1},
                         "unexpected end of file",
                         "the program may be missing 'end' or '.'"});
      eof_reported_ = true;
    }
    return eof_sentinel;
  }
  return tokens_.at(position_);
}

const lexer::Token& Parser::peek(int ahead) const {
  if (position_ + static_cast<size_t>(ahead) >= tokens_.size()) {
    return eof_sentinel;
  }
  return tokens_.at(position_ + ahead);
}

ParsePtr Parser::consume(lexer::TokenType expected, const char* context) {
  if (is_done()) {
    return std::make_unique<ParseNode>(NodeType::Error);
  }

  if (current().type != expected) {
    std::string msg = "expected ";
    msg += lexer::toString(expected);
    if (context) {
      msg += ' ';
      msg += context;
    }
    msg += ", found ";
    msg += formatToken(current());

    diagnoser_.report(
        {diag::Phase::PARSER,
         diag::Level::ERROR,
         {current().line_num, current().col_num,
          static_cast<int>(std::max(size_t{1}, current().lexeme.size()))},
         msg,
         ""});

    position_++;
    return std::make_unique<ParseNode>(NodeType::Error);
  }

  return std::make_unique<ParseNode>(NodeType::TokenNode,
                                     tokens_.at(position_++));
}

const std::string Parser::formatToken(const lexer::Token& t) const {
  auto msg = std::string(lexer::toString(t.type));
  if (!t.lexeme.empty() && (t.type == lexer::TokenType::IDENT ||
                            t.type == lexer::TokenType::INTCON ||
                            t.type == lexer::TokenType::REALCON ||
                            t.type == lexer::TokenType::CHARCON ||
                            t.type == lexer::TokenType::STRING ||
                            t.type == lexer::TokenType::INVALID)) {
    msg += " '";
    msg += t.lexeme;
    msg += '\'';
  }
  return msg;
}

void Parser::sync(std::initializer_list<lexer::TokenType> stops) {
  while (!is_done()) {
    for (auto s : stops) {
      if (current().type == s) return;
    }
    position_++;
  }
}

void Parser::ensureProgress(size_t previous_position,
                            const std::string& context) {
  if (position_ != previous_position || is_done()) {
    return;
  }

  diagnoser_.report(
      {diag::Phase::PARSER,
       diag::Level::ERROR,
       {current().line_num, current().col_num,
        static_cast<int>(std::max(size_t{1}, current().lexeme.size()))},
       "parser could not recover while parsing " + context + " near " +
           formatToken(current()),
       "skipping this token to continue parsing"});
  position_++;
}

}  // namespace parser
