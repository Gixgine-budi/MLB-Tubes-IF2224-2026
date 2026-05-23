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

int SemanticAnalyzer::get_base_type(const std::string& type_name) {
    auto entry = sym_table.lookup(type_name);
    return entry ? entry->idx : 0;
}

bool SemanticAnalyzer::isAssignmentCompatible(int target_type, int expr_type) {
    if (target_type == expr_type) return true; // Aturan 1: Type yang sama
    
    int real_type = get_base_type("real");
    int int_type = get_base_type("integer");
    
    if (target_type == real_type && expr_type == int_type) {
        return true;
    }
    
    return false;
}
}  // namespace semantic