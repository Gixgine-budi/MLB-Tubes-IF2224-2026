#include <memory>

#include "arion_exceptions.hpp"
#include "lexer/token.hpp"
#include "parser.hpp"
#include "parser/parse_node.hpp"

namespace parser {

using lexer::TokenType;

namespace {

bool case_label_starts_here(const lexer::Token& tok) {
  switch (tok.type) {
    case TokenType::INTCON:
    case TokenType::REALCON:
    case TokenType::CHARCON:
    case TokenType::STRING:
    case TokenType::PLUS:
    case TokenType::MINUS:
    case TokenType::IDENT:
      return true;
    default:
      return false;
  }
}

bool index_element_starts_here(const lexer::Token& tok) {
  switch (tok.type) {
    case TokenType::INTCON:
    case TokenType::CHARCON:
    case TokenType::IDENT:
      return true;
    default:
      return false;
  }
}

}  // namespace

ParsePtr Parser::parseCompoundStatement() {
  auto node = std::make_unique<ParseNode>(NodeType::CompoundStatement);
  node->addChild(consume(TokenType::BEGINSY));
  node->addChild(parseStatementList());
  node->addChild(consume(TokenType::ENDSY));
  return node;
}

ParsePtr Parser::parseStatementList() {
  auto node = std::make_unique<ParseNode>(NodeType::StatementList);
  while (current().type != TokenType::ENDSY &&
         current().type != TokenType::UNTILSY) {
    node->addChild(parseStatement());
    if (current().type == TokenType::SEMICOLON) {
      node->addChild(consume(TokenType::SEMICOLON));
      continue;
    }
    if (current().type == TokenType::ENDSY ||
        current().type == TokenType::UNTILSY) {
      break;
    }
    throw UnexpectedTokenException(filename_, TokenType::SEMICOLON,
                                   current());
  }
  return node;
}

ParsePtr Parser::parseStatement() {
  auto node = std::make_unique<ParseNode>(NodeType::Statement);
  switch (current().type) {
    case TokenType::IFSY:
      node->addChild(parseIfStatement());
      break;
    case TokenType::CASESY:
      node->addChild(parseCaseStatement());
      break;
    case TokenType::WHILESY:
      node->addChild(parseWhileStatement());
      break;
    case TokenType::REPEATSY:
      node->addChild(parseRepeatStatement());
      break;
    case TokenType::FORSY:
      node->addChild(parseForStatement());
      break;
    case TokenType::IDENT: {
      // ident lparent begins a call; otherwise parse a variable and require
      // becomes (:=) on the LHS.
      if (!is_done() && peek(1).type == TokenType::LPARENT) {
        node->addChild(parseFunctionCall());
      } else {
        ParsePtr var = parseVariable();
        if (current().type == TokenType::BECOMES) {
          auto asn =
              std::make_unique<ParseNode>(NodeType::AssignmentStatement);
          asn->addChild(std::move(var));
          asn->addChild(consume(TokenType::BECOMES));
          asn->addChild(parseExpression());
          node->addChild(std::move(asn));
        } else {
          throw UnexpectedTokenException(filename_, TokenType::BECOMES,
                                         current());
        }
      }
      break;
    }
    case TokenType::SEMICOLON:
    case TokenType::ENDSY:
    case TokenType::UNTILSY:
    case TokenType::ELSESY:
      break;
    default:
      throw UnexpectedTokenException(filename_, TokenType::IDENT, current());
  }
  return node;
}

ParsePtr Parser::parseVariable() {
  if (current().type != TokenType::IDENT) {
    throw UnexpectedTokenException(filename_, TokenType::IDENT, current());
  }
  auto node = std::make_unique<ParseNode>(NodeType::Variable);
  node->addChild(consume(TokenType::IDENT));
  while (current().type == TokenType::LBRACK ||
         current().type == TokenType::PERIOD) {
    node->addChild(parseComponentVariable());
  }
  return node;
}

ParsePtr Parser::parseComponentVariable() {
  auto node = std::make_unique<ParseNode>(NodeType::ComponentVariable);
  if (current().type == TokenType::LBRACK) {
    node->addChild(consume(TokenType::LBRACK));
    node->addChild(parseIndexList());
    node->addChild(consume(TokenType::RBRACK));
  } else if (current().type == TokenType::PERIOD) {
    node->addChild(consume(TokenType::PERIOD));
    node->addChild(consume(TokenType::IDENT));
  } else {
    throw UnexpectedTokenException(filename_, TokenType::LBRACK, current());
  }
  return node;
}

ParsePtr Parser::parseIndexList() {
  auto node = std::make_unique<ParseNode>(NodeType::IndexList);
  if (!index_element_starts_here(current())) {
    throw UnexpectedTokenException(filename_, TokenType::INTCON, current());
  }
  {
    const TokenType ty = current().type;
    node->addChild(consume(ty));
  }
  while (current().type == TokenType::COMMA) {
    node->addChild(consume(TokenType::COMMA));
    if (!index_element_starts_here(current())) {
      throw UnexpectedTokenException(filename_, TokenType::INTCON, current());
    }
    const TokenType ty = current().type;
    node->addChild(consume(ty));
  }
  return node;
}

ParsePtr Parser::parseAssignmentStatement() {
  auto node = std::make_unique<ParseNode>(NodeType::AssignmentStatement);
  node->addChild(parseVariable());
  node->addChild(consume(TokenType::BECOMES));
  node->addChild(parseExpression());
  return node;
}

ParsePtr Parser::parseIfStatement() {
  auto node = std::make_unique<ParseNode>(NodeType::IfStatement);
  node->addChild(consume(TokenType::IFSY));
  node->addChild(parseExpression());
  node->addChild(consume(TokenType::THENSY));
  node->addChild(parseStatement());
  if (current().type == TokenType::ELSESY) {
    node->addChild(consume(TokenType::ELSESY));
    node->addChild(parseStatement());
  }
  return node;
}

ParsePtr Parser::parseCaseStatement() {
  auto node = std::make_unique<ParseNode>(NodeType::CaseStatement);
  node->addChild(consume(TokenType::CASESY));
  node->addChild(parseExpression());
  node->addChild(consume(TokenType::OFSY));
  node->addChild(parseCaseBlock());
  node->addChild(consume(TokenType::ENDSY));
  return node;
}

ParsePtr Parser::parseCaseBlock() {
  auto node = std::make_unique<ParseNode>(NodeType::CaseBlock);
  auto parse_one_case_constant = [this]() {
    auto c = std::make_unique<ParseNode>(NodeType::Constant);
    const lexer::Token& t = current();
    if (t.type == TokenType::CHARCON || t.type == TokenType::STRING) {
      c->addChild(consume(t.type));
      return c;
    }
    if (t.type == TokenType::PLUS || t.type == TokenType::MINUS) {
      c->addChild(consume(t.type));
    }
    const lexer::Token& u = current();
    if (u.type == TokenType::INTCON || u.type == TokenType::REALCON ||
        u.type == TokenType::IDENT) {
      c->addChild(consume(u.type));
      return c;
    }
    throw UnexpectedTokenException(filename_, TokenType::INTCON, u);
  };
  node->addChild(parse_one_case_constant());
  while (current().type == TokenType::COMMA) {
    node->addChild(consume(TokenType::COMMA));
    node->addChild(parse_one_case_constant());
  }
  node->addChild(consume(TokenType::COLON));
  node->addChild(parseStatement());
  while (current().type == TokenType::SEMICOLON) {
    node->addChild(consume(TokenType::SEMICOLON));
    if (current().type == TokenType::ENDSY) {
      break;
    }
    if (case_label_starts_here(current())) {
      node->addChild(parseCaseBlock());
    } else {
      break;
    }
  }
  return node;
}

ParsePtr Parser::parseWhileStatement() {
  auto node = std::make_unique<ParseNode>(NodeType::WhileStatement);
  node->addChild(consume(TokenType::WHILESY));
  node->addChild(parseExpression());
  node->addChild(consume(TokenType::DOSY));
  node->addChild(parseStatement());
  return node;
}

ParsePtr Parser::parseRepeatStatement() {
  auto node = std::make_unique<ParseNode>(NodeType::RepeatStatement);
  node->addChild(consume(TokenType::REPEATSY));
  node->addChild(parseStatementList());
  node->addChild(consume(TokenType::UNTILSY));
  node->addChild(parseExpression());
  return node;
}

ParsePtr Parser::parseForStatement() {
  auto node = std::make_unique<ParseNode>(NodeType::ForStatement);
  node->addChild(consume(TokenType::FORSY));
  node->addChild(consume(TokenType::IDENT));
  node->addChild(consume(TokenType::BECOMES));
  node->addChild(parseExpression());
  if (current().type == TokenType::TOSY) {
    node->addChild(consume(TokenType::TOSY));
  } else if (current().type == TokenType::DOWNTOSY) {
    node->addChild(consume(TokenType::DOWNTOSY));
  } else {
    throw UnexpectedTokenException(filename_, TokenType::TOSY, current());
  }
  node->addChild(parseExpression());
  node->addChild(consume(TokenType::DOSY));
  node->addChild(parseStatement());
  return node;
}

ParsePtr Parser::parseFunctionCall() {
  auto node = std::make_unique<ParseNode>(NodeType::FunctionCall);
  node->addChild(consume(TokenType::IDENT));
  node->addChild(consume(TokenType::LPARENT));
  if (current().type != TokenType::RPARENT) {
    node->addChild(parseParameterList());
  }
  node->addChild(consume(TokenType::RPARENT));
  return node;
}

ParsePtr Parser::parseParameterList() {
  auto node = std::make_unique<ParseNode>(NodeType::ParameterList);
  node->addChild(parseExpression());
  while (current().type == TokenType::COMMA) {
    node->addChild(consume(TokenType::COMMA));
    node->addChild(parseExpression());
  }
  return node;
}

}  // namespace parser
