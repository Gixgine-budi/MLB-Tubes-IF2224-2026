#include "semantic/semantic_analyzer.hpp"
#include "ast/expr_nodes.hpp"
#include <iostream>
#include <algorithm>

namespace semantic {

// Helper fungsi untuk mendapatkan index tipe dasar dari symbol table
static int get_base_type(SymbolTable& sym_table, const std::string& type_name) {
    auto entry = sym_table.lookup(type_name);
    return entry ? entry->idx : 0;
}

void SemanticAnalyzer::visit(ast::NumberNode &node) {
    // Jika is_real true, set tipe ke Real, jika tidak set ke Integer
    node.expression_type = node.is_real ? get_base_type(sym_table, "real") : get_base_type(sym_table, "integer");
}

void SemanticAnalyzer::visit(ast::StringNode &node) {
    node.expression_type = get_base_type(sym_table, "string");
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
    // Traverse anak secara Depth-First Search
    node.left->accept(*this);
    node.right->accept(*this);

    int l_type = node.left->expression_type;
    int r_type = node.right->expression_type;
    std::string op = node.op.lexeme;
    std::transform(op.begin(), op.end(), op.begin(), ::tolower);

    int int_type = get_base_type(sym_table, "integer");
    int real_type = get_base_type(sym_table, "real");
    int bool_type = get_base_type(sym_table, "boolean");
    int str_type = get_base_type(sym_table, "string");
    int char_type = get_base_type(sym_table, "char");

    // 1. Operator Aritmatika (+, -, *)
    if (op == "+" || op == "-" || op == "*") {
        if ((l_type == int_type || l_type == real_type) && (r_type == int_type || r_type == real_type)) {
            // Jika salah satu real, hasilnya real. Jika keduanya int, hasilnya int.
            node.expression_type = (l_type == real_type || r_type == real_type) ? real_type : int_type;
        } else {
            std::cerr << "[Semantic Error] Operand untuk '" << op << "' harus bertipe numerik.\n";
            node.expression_type = 0;
        }
    }
    // 2. Pembagian Real (/)
    else if (op == "/") {
        if ((l_type == int_type || l_type == real_type) && (r_type == int_type || r_type == real_type)) {
            node.expression_type = real_type; // '/' selalu menghasilkan real
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
        // Asumsi relasional memperbolehkan kompatibilitas numerik (int vs real)
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
        checkTypeCompatibility(get_base_type(sym_table, "boolean"), node.expr->expression_type, "Unary NOT");
        node.expression_type = get_base_type(sym_table, "boolean");
    } 
    else if (op == "+" || op == "-") {
        int int_type = get_base_type(sym_table, "integer");
        int real_type = get_base_type(sym_table, "real");
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
    
    // Visit argumen untuk type checking (Bisa ditambahkan pengecekan jumlah argumen via btab)
    for (auto &arg : node.args) {
        arg->accept(*this);
    }
}

void SemanticAnalyzer::visit(ast::ArrayAccessNode &node) {
    node.array_expr->accept(*this);
    
    for (auto &idx : node.indices) {
        idx->accept(*this);
        // Indeks array harus berupa integer atau simple type (Bukan Real berdasarkan spesifikasi)
        if (idx->expression_type == get_base_type(sym_table, "real")) {
             std::cerr << "[Semantic Error] Array index tidak boleh bertipe Real.\n";
        }
    }
    
    // Mengekstrak element type (etyp) menggunakan data dari tab dan atab yang dibuat Akmal
    int array_type_idx = node.array_expr->expression_type;
    if (array_type_idx > 0) {
        const auto& type_entry = sym_table.getTabEntry(array_type_idx);
        if (type_entry.ref > 0) { // Pointer ke atab
            const auto& array_info = sym_table.getAtabEntry(type_entry.ref);
            node.expression_type = array_info.etyp; // Mengambil tipe dari elemen array
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
        
        // Membuka scope record dari btab (disimpan di type_entry.ref)
        if (type_entry.ref > 0) {
            const auto& block_info = sym_table.getBtabEntry(type_entry.ref);
            int current_link = block_info.last;
            
            // Loop menelusuri field record (Linked list mundur via link attribute)
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