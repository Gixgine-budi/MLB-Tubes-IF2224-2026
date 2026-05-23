#include "semantic/semantic_analyzer.hpp"
#include "ast/stmt_nodes.hpp"
#include <iostream>

namespace semantic {

static int get_base_type(SymbolTable& sym_table, const std::string& type_name) {
    auto entry = sym_table.lookup(type_name);
    return entry ? entry->idx : 0;
}

static bool isAssignmentCompatible(SymbolTable& sym_table, int target_type, int expr_type) {
    if (target_type == expr_type) return true; // Aturan 1: Type yang sama
    
    int real_type = get_base_type(sym_table, "real");
    int int_type = get_base_type(sym_table, "integer");
    
    if (target_type == real_type && expr_type == int_type) {
        return true;
    }
    return false;
}

void SemanticAnalyzer::visit(ast::AssignNode &node) {
    node.target->accept(*this);
    node.expr->accept(*this);
    
    int target_type = node.target->expression_type;
    int expr_type = node.expr->expression_type;

    if (target_type == 0 || expr_type == 0) return; // Error cascade prevention

    if (!isAssignmentCompatible(sym_table, target_type, expr_type)) {
        std::cerr << "[Semantic Error] Type Incompatible untuk Assignment.\n";
    }
}

void SemanticAnalyzer::visit(ast::IfNode &node) {
    node.condition->accept(*this);
    
    int bool_type = get_base_type(sym_table, "boolean");
    if (node.condition->expression_type != bool_type) {
        std::cerr << "[Semantic Error] Kondisi IF wajib bertipe Boolean.\n";
    }

    node.then_branch->accept(*this);
    if (node.else_branch) {
        node.else_branch->accept(*this);
    }
}

void SemanticAnalyzer::visit(ast::WhileNode &node) {
    node.condition->accept(*this);

    int bool_type = get_base_type(sym_table, "boolean");
    if (node.condition->expression_type != bool_type) {
        std::cerr << "[Semantic Error] Kondisi WHILE wajib bertipe Boolean.\n";
    }

    node.body->accept(*this);
}

void SemanticAnalyzer::visit(ast::RepeatNode &node) {
    for (auto &stmt : node.statements) {
        stmt->accept(*this);
    }
    
    node.condition->accept(*this);
    int bool_type = get_base_type(sym_table, "boolean");
    if (node.condition->expression_type != bool_type) {
        std::cerr << "[Semantic Error] Kondisi UNTIL wajib bertipe Boolean.\n";
    }
}

void SemanticAnalyzer::visit(ast::ForNode &node) {
    auto entry = sym_table.lookup(node.iterator.lexeme);
    if (!entry) {
        std::cerr << "[Semantic Error] Undeclared iterator variable: " << node.iterator.lexeme << "\n";
    }

    node.initial->accept(*this);
    node.final->accept(*this);

    if (entry && node.initial->expression_type != entry->type) {
        std::cerr << "[Semantic Error] Nilai awal iterasi tidak compatible dengan tipe iterator.\n";
    }
    if (entry && node.final->expression_type != entry->type) {
        std::cerr << "[Semantic Error] Nilai akhir iterasi tidak compatible dengan tipe iterator.\n";
    }

    node.body->accept(*this);
}

void SemanticAnalyzer::visit(ast::ProcCallNode &node) {
    auto entry = sym_table.lookup(node.id.lexeme);
    if (!entry) {
        std::cerr << "[Semantic Error] Undeclared procedure: " << node.id.lexeme << "\n";
    } else if (entry->obj != ObjClass::Procedure) {
        std::cerr << "[Semantic Error] Identifier '" << node.id.lexeme << "' is not a procedure.\n";
    }

    // Melakukan Type Checking pada Argumen Parameter vs Caller Arguments (Bisa menggunakan btab)
    for (auto &arg : node.args) {
        arg->accept(*this);
    }
}

void SemanticAnalyzer::visit(ast::CompoundStmtNode &node) {
    // Traverse seluruh statement berurutan
    for (auto &stmt : node.statements) {
        stmt->accept(*this);
    }
}

} // namespace semantic