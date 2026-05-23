#include "semantic/semantic_analyzer.hpp"
#include "ast/stmt_nodes.hpp"
#include <iostream>
#include <algorithm>

namespace semantic {

void SemanticAnalyzer::visit(ast::AssignNode &node) {
    node.target->accept(*this);
    node.expr->accept(*this);
    
    int target_type = node.target->expression_type;
    int expr_type = node.expr->expression_type;

    if (target_type == 0 || expr_type == 0) return; // Mencegah error beruntun

    if (!isAssignmentCompatible(target_type, expr_type)) {
        std::cerr << "[Semantic Error] Type Incompatible untuk Assignment.\n";
    }
}

void SemanticAnalyzer::visit(ast::IfNode &node) {
    node.condition->accept(*this);
    
    if (node.condition->expression_type != get_base_type("boolean")) {
        std::cerr << "[Semantic Error] Kondisi IF wajib bertipe Boolean.\n";
    }

    node.then_branch->accept(*this);
    if (node.else_branch) {
        node.else_branch->accept(*this);
    }
}

void SemanticAnalyzer::visit(ast::WhileNode &node) {
    node.condition->accept(*this);

    if (node.condition->expression_type != get_base_type("boolean")) {
        std::cerr << "[Semantic Error] Kondisi WHILE wajib bertipe Boolean.\n";
    }

    node.body->accept(*this);
}

void SemanticAnalyzer::visit(ast::RepeatNode &node) {
    for (auto &stmt : node.statements) {
        stmt->accept(*this);
    }
    
    node.condition->accept(*this);
    if (node.condition->expression_type != get_base_type("boolean")) {
        std::cerr << "[Semantic Error] Kondisi UNTIL wajib bertipe Boolean.\n";
    }
}

void SemanticAnalyzer::visit(ast::ForNode &node) {
    auto entry = sym_table.lookup(node.iterator.lexeme);
    if (!entry) {
        std::cerr << "[Semantic Error] Undeclared iterator variable: " << node.iterator.lexeme << "\n";
    } else {
        // Validasi ekstra: Iterator WAJIB bertipe Integer (Ordinal)
        int int_type = get_base_type("integer");
        if (entry->type != int_type) {
            std::cerr << "[Semantic Error] Tipe iterator pada FOR loop wajib bertipe Integer.\n";
        }
    }

    node.initial->accept(*this);
    node.final->accept(*this);

    if (entry && !isAssignmentCompatible(entry->type, node.initial->expression_type)) {
        std::cerr << "[Semantic Error] Nilai awal iterasi tidak assignment-compatible dengan tipe iterator.\n";
    }
    if (entry && !isAssignmentCompatible(entry->type, node.final->expression_type)) {
        std::cerr << "[Semantic Error] Nilai akhir iterasi tidak assignment-compatible dengan tipe iterator.\n";
    }

    node.body->accept(*this);
}

void SemanticAnalyzer::visit(ast::ProcCallNode &node) {
    std::string proc_name = node.id.lexeme;
    std::transform(proc_name.begin(), proc_name.end(), proc_name.begin(), ::tolower);

    if (proc_name == "writeln" || proc_name == "readln" || proc_name == "write" || proc_name == "read") {
        for (auto &arg : node.args) {
            arg->accept(*this);
        }
        return; 
    }

    auto entry = sym_table.lookup(node.id.lexeme);
    if (!entry) {
        std::cerr << "[Semantic Error] Undeclared procedure: " << node.id.lexeme << "\n";
    } else if (entry->obj != ObjClass::Procedure) {
        std::cerr << "[Semantic Error] Identifier '" << node.id.lexeme << "' is not a procedure.\n";
    }

    for (auto &arg : node.args) {
        arg->accept(*this);
    }
}

void SemanticAnalyzer::visit(ast::CompoundStmtNode &node) {

    for (auto &stmt : node.statements) {
        stmt->accept(*this);
    }
}

} // namespace semantic