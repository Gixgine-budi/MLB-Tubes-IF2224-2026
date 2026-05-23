#include "ast/stmt_nodes.hpp"
#include "parser/parse_node.hpp"
#include "semantic/sdt_builder.hpp"

namespace semantic {

Ptr<ast::StmtNode> SDTBuilder::buildStatement(const parser::ParseNode &node) {
  if (node.children().empty()) return nullptr;

  auto &child = *node.children()[0];
  switch (child.type()) {
    case parser::NodeType::AssignmentStatement:
      return buildAssign(child);
    case parser::NodeType::IfStatement:
      return buildIf(child);
    case parser::NodeType::WhileStatement:
      return buildWhile(child);
    case parser::NodeType::RepeatStatement:
      return buildRepeat(child);
    case parser::NodeType::ForStatement:
      return buildFor(child);
    case parser::NodeType::FunctionCall:
      return buildProcCall(child);
    case parser::NodeType::CompoundStatement:
      return buildCompoundStmt(child);
    default:
      return nullptr;
  }
}

Ptr<ast::CompoundStmtNode> SDTBuilder::buildCompoundStmt(
    const parser::ParseNode &node) {
  std::vector<Ptr<ast::StmtNode>> stmts;
  // CompoundStatement -> BEGINSY StatementList ENDSY
  if (node.children().size() < 2)
    return std::make_unique<ast::CompoundStmtNode>(std::move(stmts));

  auto &stmt_list = *node.children()[1];
  for (const auto &child : stmt_list.children()) {
    if (child->type() == parser::NodeType::Statement) {
      auto stmt = buildStatement(*child);
      if (stmt) stmts.push_back(std::move(stmt));
    }
  }
  return std::make_unique<ast::CompoundStmtNode>(std::move(stmts));
}

Ptr<ast::AssignNode> SDTBuilder::buildAssign(const parser::ParseNode &node) {
  if (node.children().size() < 3) return nullptr;
  auto target = buildVariableAccess(*node.children()[0]);
  auto expr = buildExpression(*node.children()[2]);
  return std::make_unique<ast::AssignNode>(std::move(target), std::move(expr));
}

Ptr<ast::IfNode> SDTBuilder::buildIf(const parser::ParseNode &node) {
  if (node.children().size() < 4) return nullptr;
  auto cond = buildExpression(*node.children()[1]);
  auto then_b = buildStatement(*node.children()[3]);
  Ptr<ast::StmtNode> else_b = nullptr;
  if (node.children().size() > 5) {
    else_b = buildStatement(*node.children()[5]);
  }
  return std::make_unique<ast::IfNode>(std::move(cond), std::move(then_b),
                                       std::move(else_b));
}

Ptr<ast::WhileNode> SDTBuilder::buildWhile(const parser::ParseNode &node) {
  if (node.children().size() < 4) return nullptr;
  auto cond = buildExpression(*node.children()[1]);
  auto body = buildStatement(*node.children()[3]);
  return std::make_unique<ast::WhileNode>(std::move(cond), std::move(body));
}

Ptr<ast::RepeatNode> SDTBuilder::buildRepeat(const parser::ParseNode &node) {
  std::vector<Ptr<ast::StmtNode>> stmts;
  if (node.children().size() < 4) return nullptr;

  auto &stmt_list = *node.children()[1];
  for (const auto &child : stmt_list.children()) {
    if (child->type() == parser::NodeType::Statement) {
      auto stmt = buildStatement(*child);
      if (stmt) stmts.push_back(std::move(stmt));
    }
  }
  auto cond = buildExpression(*node.children()[3]);
  return std::make_unique<ast::RepeatNode>(std::move(stmts), std::move(cond));
}

Ptr<ast::ForNode> SDTBuilder::buildFor(const parser::ParseNode &node) {
  if (node.children().size() < 8) return nullptr;
  lexer::Token iter = node.children()[1]->token().value();
  auto start = buildExpression(*node.children()[3]);
  bool is_down =
      (node.children()[4]->token().value().type == lexer::TokenType::DOWNTOSY);
  auto end = buildExpression(*node.children()[5]);
  auto body = buildStatement(*node.children()[7]);

  return std::make_unique<ast::ForNode>(iter, std::move(start), std::move(end),
                                        is_down, std::move(body));
}

Ptr<ast::ProcCallNode> SDTBuilder::buildProcCall(
    const parser::ParseNode &node) {
  if (node.children().empty()) return nullptr;

  lexer::Token id_tok = node.children()[0]->token().value();
  std::vector<Ptr<ast::ExprNode>> args;

  if (node.children().size() > 1 &&
      node.children()[1]->type() == parser::NodeType::ParameterList) {
    const auto &param_list = node.children()[1];
    for (const auto &p : param_list->children()) {
      if (p->type() == parser::NodeType::Expression) {
        args.push_back(buildExpression(*p));
      }
    }
  }

  return std::make_unique<ast::ProcCallNode>(id_tok, std::move(args));
}

}  // namespace semantic