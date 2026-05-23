#include "semantic/semantic_analyzer.hpp"
#include "ast/expr_nodes.hpp"
#include <iostream>
#include <algorithm>

namespace semantic {

void SemanticAnalyzer::visit(ast::NumberNode &node) {
    node.expression_type = node.is_real ? get_base_type("real") : get_base_type("integer");
}

void SemanticAnalyzer::visit(ast::StringNode &node) {
    if (node.val.type == lexer::TokenType::CHARCON) {
        node.expression_type = get_base_type("char");
    } else {
        node.expression_type = get_base_type("string");
    }
}

void SemanticAnalyzer::visit(ast::IdentNode &node) {
    auto entry = sym_table.lookup(node.id.lexeme);
    if (!entry) {
        std::cerr << "[Semantic Error] Undeclared identifier: " << node.id.lexeme << " at line " << node.id.line_num << "\n";
        node.expression_type = 0; // Unknown
    } else {
        node.expression_type = entry->type;
    }
}

void SemanticAnalyzer::visit(ast::BinOpNode &node) {
    node.left->accept(*this);
    node.right->accept(*this);

    int l_type = node.left->expression_type;
    int r_type = node.right->expression_type;
    std::string op = node.op.lexeme;
    std::transform(op.begin(), op.end(), op.begin(), ::tolower);

    int int_type = get_base_type("integer");
    int real_type = get_base_type("real");
    int bool_type = get_base_type("boolean");

    // 1. Operator Aritmatika (+, -, *)
    if (op == "+" || op == "-" || op == "*") {
        if ((l_type == int_type || l_type == real_type) && (r_type == int_type || r_type == real_type)) {
            node.expression_type = (l_type == real_type || r_type == real_type) ? real_type : int_type;
        } else {
            std::cerr << "[Semantic Error] Operand untuk '" << op << "' harus bertipe numerik.\n";
            node.expression_type = 0;
        }
    }
    // 2. Pembagian Real (/)
    else if (op == "/") {
        if ((l_type == int_type || l_type == real_type) && (r_type == int_type || r_type == real_type)) {
            node.expression_type = real_type;
        } else {
            std::cerr << "[Semantic Error] Operand untuk '/' harus bertipe numerik.\n";
            node.expression_type = 0;
        }
    }
    // 3. Pembagian Integer (div, mod)
    else if (op == "div" || op == "mod") {
        if (l_type == int_type && r_type == int_type) {
            node.expression_type = int_type;
        } else {
            std::cerr << "[Semantic Error] Operand untuk '" << op << "' wajib bertipe integer.\n";
            node.expression_type = 0;
        }
    }
    // 4. Operator Relasional (=, <>, <, >, <=, >=)
    else if (op == "=" || op == "<>" || op == "<" || op == ">" || op == "<=" || op == ">=") {
        if (l_type == r_type || 
           ((l_type == int_type || l_type == real_type) && (r_type == int_type || r_type == real_type))) {
            node.expression_type = bool_type;
        } else {
            std::cerr << "[Semantic Error] Tipe tidak kompatibel untuk operasi perbandingan '" << op << "'.\n";
            node.expression_type = 0;
        }
    }
    // 5. Operator Logika (and, or)
    else if (op == "and" || op == "or") {
        if (l_type == bool_type && r_type == bool_type) {
            node.expression_type = bool_type;
        } else {
            std::cerr << "[Semantic Error] Operand untuk operator logika '" << op << "' harus bertipe boolean.\n";
            node.expression_type = 0;
        }
    } else {
        node.expression_type = 0;
    }
}

void SemanticAnalyzer::visit(ast::UnaryOpNode &node) {
    node.expr->accept(*this);
    std::string op = node.op.lexeme;
    std::transform(op.begin(), op.end(), op.begin(), ::tolower);
    
    if (op == "not") {
        if (node.expr->expression_type != get_base_type("boolean")) {
            std::cerr << "[Semantic Error] Operand untuk NOT harus bertipe boolean.\n";
        }
        node.expression_type = get_base_type("boolean");
    } 
    else if (op == "+" || op == "-") {
        int int_type = get_base_type("integer");
        int real_type = get_base_type("real");
        if (node.expr->expression_type == int_type || node.expr->expression_type == real_type) {
            node.expression_type = node.expr->expression_type;
        } else {
            std::cerr << "[Semantic Error] Unary sign '" << op << "' hanya berlaku untuk tipe numerik.\n";
            node.expression_type = 0;
        }
    }
}

void SemanticAnalyzer::visit(ast::FuncCallNode &node) {
    auto entry = sym_table.lookup(node.id.lexeme);
    if (!entry) {
        std::cerr << "[Semantic Error] Undeclared function: " << node.id.lexeme << "\n";
        node.expression_type = 0;
    } else if (entry->obj != ObjClass::Function) {
        std::cerr << "[Semantic Error] Identifier '" << node.id.lexeme << "' is not a function.\n";
        node.expression_type = 0;
    } else {
        node.expression_type = entry->type;
    }
    
    // Visit argumen untuk type checking
    for (auto &arg : node.args) {
        arg->accept(*this);
    }
}

void SemanticAnalyzer::visit(ast::ArrayAccessNode &node) {
    node.array_expr->accept(*this);
    
    for (auto &idx : node.indices) {
        idx->accept(*this);
        // Indeks array tidak boleh bertipe real
        if (idx->expression_type == get_base_type("real")) {
             std::cerr << "[Semantic Error] Array index tidak boleh bertipe Real.\n";
        }
    }
    
    // Ekstraksi etyp menggunakan data dari tab dan atab
    int array_type_idx = node.array_expr->expression_type;
    if (array_type_idx > 0) {
        const auto& type_entry = sym_table.getTabEntry(array_type_idx);
        if (type_entry.ref > 0) { 
            const auto& array_info = sym_table.getAtabEntry(type_entry.ref);
            node.expression_type = array_info.etyp;
            return;
        }
    }
    
    std::cerr << "[Semantic Error] Invalid array access.\n";
    node.expression_type = 0; 
}

void SemanticAnalyzer::visit(ast::RecordAccessNode &node) {
    node.record_expr->accept(*this);
    
    int record_type_idx = node.record_expr->expression_type;
    if (record_type_idx > 0) {
        const auto& type_entry = sym_table.getTabEntry(record_type_idx);
        
        // Membuka scope record dari btab
        if (type_entry.ref > 0) {
            const auto& block_info = sym_table.getBtabEntry(type_entry.ref);
            int current_link = block_info.last;
            
            // Mencari field (Linked list)
            while (current_link > 0) {
                const auto& field_entry = sym_table.getTabEntry(current_link);
                if (field_entry.id == node.field.lexeme) {
                    node.expression_type = field_entry.type; // Field ketemu
                    return;
                }
                current_link = field_entry.link;
            }
            std::cerr << "[Semantic Error] Record tidak memiliki field '" << node.field.lexeme << "'.\n";
        }
    }
    node.expression_type = 0; 
}

} // namespace semantic