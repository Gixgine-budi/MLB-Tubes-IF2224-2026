#include "semantic/semantic_analyzer.hpp"

namespace semantic {

void SemanticAnalyzer::visit(ast::BinOpNode &node) {}

void SemanticAnalyzer::visit(ast::UnaryOpNode &node) {}

void SemanticAnalyzer::visit(ast::NumberNode &node) {}

void SemanticAnalyzer::visit(ast::StringNode &node) {}

void SemanticAnalyzer::visit(ast::IdentNode &node) {}

void SemanticAnalyzer::visit(ast::FuncCallNode &node) {}

void SemanticAnalyzer::visit(ast::ArrayAccessNode &node) {}

void SemanticAnalyzer::visit(ast::RecordAccessNode &node) {}

}  // namespace semantic