#include "semantic/sdt_builder.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "ast/ast_node.hpp"
#include "diagnoser/diagnoser.hpp"
#include "diagnoser/diagnostic.hpp"
#include "parser/parse_node.hpp"

namespace semantic {

namespace {

const char *rootType(const ast::AstNode &node) {
  if (dynamic_cast<const ast::ProgramNode *>(&node) != nullptr) {
    return "ProgramNode";
  }
  if (dynamic_cast<const ast::BlockNode *>(&node) != nullptr) {
    return "BlockNode";
  }
  if (dynamic_cast<const ast::StmtNode *>(&node) != nullptr) {
    return "StmtNode";
  }
  if (dynamic_cast<const ast::ExprNode *>(&node) != nullptr) {
    return "ExprNode";
  }
  return "AstNode";
}

}  // namespace

SDTBuilder::SDTBuilder(const parser::ParseNode &parse_root,
                       diag::Diagnoser &diagnoser)
    : parse_root_(parse_root), diagnoser_(diagnoser) {}

void SDTBuilder::build() {
  built_ = true;
  has_errors_ = false;
  ast_root_.reset();

  if (parse_root_.type() != parser::NodeType::Program) {
    reportBuildError(parse_root_, "expected Program parse node as SDT root");
    return;
  }

  auto program = buildProgram(parse_root_);
  if (program == nullptr) {
    reportBuildError(parse_root_,
                     "failed to translate parse tree into Program AST node");
    return;
  }

  ast_root_ = std::move(program);
}

void SDTBuilder::print(bool ascii) const { print(std::cout, ascii); }

void SDTBuilder::print(std::ostream &os, bool ascii) const {
  (void)ascii;

  if (!built_) {
    os << "SDTBuilder: build() has not been called.\n";
    return;
  }

  if (ast_root_ == nullptr) {
    os << "SDTBuilder: AST root is null.\n";
    return;
  }

  if (const auto *program =
          dynamic_cast<const ast::ProgramNode *>(ast_root_.get())) {
    os << "ProgramNode(name: '" << program->identifier.lexeme << "')\n";

    if (program->block != nullptr) {
      const size_t decl_count = program->block->declarations.size();
      const size_t stmt_count =
          program->block->compound_stmt != nullptr
              ? program->block->compound_stmt->statements.size()
              : 0;

      os << "  declarations: " << decl_count << "\n";
      os << "  statements: " << stmt_count << "\n";
    }

    return;
  }

  os << rootType(*ast_root_) << "\n";
}

const ast::AstNode &SDTBuilder::getAst() const {
  if (ast_root_ == nullptr) {
    throw std::logic_error(
        "SDTBuilder::getAst() called before a successful build()");
  }
  return *ast_root_;
}

void SDTBuilder::reportBuildError(const parser::ParseNode &node,
                                  const std::string &message,
                                  const std::string &hint) {
  diag::Source source{};
  if (const lexer::Token *tok = firstToken(node)) {
    source.line = tok->line_num;
    source.col = tok->col_num;
    source.length = static_cast<int>(std::max<size_t>(1, tok->lexeme.size()));
  }

  diagnoser_.report(
      {diag::Phase::SEMANTIC, diag::Level::ERROR, source, message, hint});
  has_errors_ = true;
}

const lexer::Token *SDTBuilder::firstToken(
    const parser::ParseNode &node) const {
  if (node.token().has_value()) {
    return &node.token().value();
  }

  for (const auto &child : node.children()) {
    if (const lexer::Token *tok = firstToken(*child)) {
      return tok;
    }
  }

  return nullptr;
}

Ptr<ast::ProgramNode> SDTBuilder::buildProgram(const parser::ParseNode &node) {
  if (node.type() != parser::NodeType::Program) {
    reportBuildError(node, "internal SDT mismatch: expected Program node");
    return nullptr;
  }

  lexer::Token program_id;
  std::vector<lexer::Token> params;
  Ptr<ast::BlockNode> block = nullptr;

  const auto &children = node.children();
  if (children.empty()) {
    reportBuildError(node, "Program node has no children");
    return nullptr;
  }

  if (children[0]->type() == parser::NodeType::ProgramHeader) {
    const auto &header_children = children[0]->children();
    for (const auto &hc : header_children) {
      if (hc->type() == parser::NodeType::TokenNode &&
          hc->token().has_value() &&
          hc->token()->type == lexer::TokenType::IDENT) {
        program_id = hc->token().value();
      }
    }
  }

  std::vector<Ptr<ast::AstNode>> declarations;
  Ptr<ast::CompoundStmtNode> comp_stmt = nullptr;

  for (size_t i = 1; i < children.size(); ++i) {
    if (children[i]->type() == parser::NodeType::DeclarationPart) {
      // Flatten returned declarations
      auto decls = buildDeclarations(*children[i]);
      for (auto &d : decls) {
        declarations.push_back(std::move(d));
      }
    } else if (children[i]->type() == parser::NodeType::CompoundStatement) {
      comp_stmt = buildCompoundStmt(*children[i]);
    }
  }

  if (comp_stmt == nullptr) {
    reportBuildError(node,
                     "Program node does not contain a valid CompoundStatement");
  }

  block = std::make_unique<ast::BlockNode>(std::move(declarations),
                                           std::move(comp_stmt));

  return std::make_unique<ast::ProgramNode>(program_id, params,
                                            std::move(block));
}

Ptr<ast::BlockNode> SDTBuilder::buildBlock(const parser::ParseNode &node) {
  if (node.type() != parser::NodeType::Block) {
    reportBuildError(node, "internal SDT mismatch: expected Block node");
    return nullptr;
  }

  std::vector<Ptr<ast::AstNode>> declarations;
  Ptr<ast::CompoundStmtNode> comp_stmt = nullptr;

  for (const auto &child : node.children()) {
    if (child->type() == parser::NodeType::DeclarationPart) {
      auto decls = buildDeclarations(*child);
      for (auto &d : decls) {
        declarations.push_back(std::move(d));
      }
    } else if (child->type() == parser::NodeType::CompoundStatement) {
      comp_stmt = buildCompoundStmt(*child);
    }
  }

  return std::make_unique<ast::BlockNode>(std::move(declarations),
                                          std::move(comp_stmt));
}

}  // namespace semantic