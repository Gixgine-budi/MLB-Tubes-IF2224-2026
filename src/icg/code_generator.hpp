#include "../ast/ast_visitor.hpp"
#include "../ast/decl_nodes.hpp"
#include "../ast/stmt_nodes.hpp"
#include "../ast/expr_nodes.hpp"
#include "../semantic/symbol_table.hpp"
#include "../lexer/token.hpp"
#include "instruction.hpp"
#include <vector>
#include <iostream>

class CodeGenerator : public ast::ASTVisitor {
private:
    std::vector<Instruction> code;
    const semantic::SymbolTable& sym_table;
    int current_level;

    int emit(OpCode op, int l, int a) {
        code.push_back({op, l, a});
        return code.size() - 1; 
    }

    // Push relative array index (index - low) onto the stack.
    bool emitArrayRelativeIndex(ast::ArrayAccessNode& node, int& level_diff,
                                int& base_adr) {
        auto* ident = dynamic_cast<ast::IdentNode*>(node.array_expr.get());
        if (ident == nullptr || ident->tab_index == 0 || node.indices.empty()) {
            return false;
        }

        const auto& var_entry = sym_table.getTabEntry(ident->tab_index);
        const auto& type_entry = sym_table.getTabEntry(var_entry.type);
        if (type_entry.type != static_cast<int>(semantic::BuiltinType::Array) ||
            type_entry.ref <= 0) {
            return false;
        }

        const auto& atab_entry = sym_table.getAtabEntry(type_entry.ref);
        level_diff = current_level - var_entry.lev;
        base_adr = var_entry.adr;

        node.indices[0]->accept(*this);
        emit(OpCode::LIT, 0, atab_entry.low);
        emit(OpCode::OPR, 0, 3);  // SUB: index - low
        return true;
    }

public:
    CodeGenerator(const semantic::SymbolTable& st) : sym_table(st), current_level(0) {}

    const std::vector<Instruction>& getCode() const { return code; }

    void printCode() const {
        for (size_t i = 0; i < code.size(); ++i) {
            std::cout << i << " " << code[i].getOpString() << " " 
                      << code[i].l << " " << code[i].a << "\n";
        }
    }

    void visit(ast::ProgramNode& node) override {
        int jmp_main = emit(OpCode::JMP, 0, 0);

        if (node.block) {
            for (auto& decl : node.block->declarations) {
                decl->accept(*this);
            }
        }

        code[jmp_main].a = code.size(); // Backpatch JMP ke awal eksekusi utama

        int main_vsze = sym_table.getBtabEntry(0).vsze; 
        emit(OpCode::INT, 0, 3 + main_vsze);

        if (node.block && node.block->compound_stmt) {
            node.block->compound_stmt->accept(*this);
        }

        emit(OpCode::RET, 0, 0);
    }

    void visit(ast::BlockNode& node) override {
        for (auto& decl : node.declarations) {
            decl->accept(*this);
        }
        if (node.compound_stmt) {
            node.compound_stmt->accept(*this);
        }
    }

    void visit(ast::CompoundStmtNode& node) override {
        for (auto& stmt : node.statements) {
            stmt->accept(*this);
        }
    }

    void visit(ast::AssignNode& node) override {
        if (auto* arr = dynamic_cast<ast::ArrayAccessNode*>(node.target.get())) {
            int level_diff = 0;
            int base_adr = 0;
            if (!emitArrayRelativeIndex(*arr, level_diff, base_adr)) {
                return;
            }
            node.expr->accept(*this);
            emit(OpCode::STX, level_diff, base_adr);
            return;
        }

        node.expr->accept(*this);

        auto* ident = dynamic_cast<ast::IdentNode*>(node.target.get());
        if (ident == nullptr || ident->tab_index == 0) {
            return;
        }
        const auto& entry = sym_table.getTabEntry(ident->tab_index);
        int level_diff = current_level - entry.lev;
        emit(OpCode::STO, level_diff, entry.adr);
    }

    void visit(ast::IfNode& node) override {
        node.condition->accept(*this);
        
        int jpc_idx = emit(OpCode::JPC, 0, 0); // Kondisi salah melompat
        
        node.then_branch->accept(*this);

        if (node.else_branch) {
            int jmp_idx = emit(OpCode::JMP, 0, 0); // Lewati Else
            code[jpc_idx].a = code.size();         // Backpatch JPC ke Else
            node.else_branch->accept(*this);
            code[jmp_idx].a = code.size();         // Backpatch JMP ke End
        } else {
            code[jpc_idx].a = code.size();         // Backpatch JPC ke End
        }
    }

    void visit(ast::WhileNode& node) override {
        int start_idx = code.size(); 
        
        node.condition->accept(*this);
        int jpc_idx = emit(OpCode::JPC, 0, 0); 
        
        node.body->accept(*this);
        
        emit(OpCode::JMP, 0, start_idx); // Loop ulang
        code[jpc_idx].a = code.size();   
    }

    void visit(ast::ProcCallNode& node) override {
        if (node.id.lexeme == "writeln" || node.id.lexeme == "write") {
            for (auto& arg : node.args) {
                arg->accept(*this);
                emit(OpCode::OPR, 0, 13); // WRT (Output)
            }
            if (node.id.lexeme == "writeln") {
                emit(OpCode::OPR, 0, 14); // WRTLN (Newline)
            }
            return;
        }

        const auto& entry = sym_table.getTabEntry(node.tab_index);
        for (auto& arg : node.args) {
            arg->accept(*this); // Push argumen
        }
        int level_diff = current_level - entry.lev;
        emit(OpCode::CAL, level_diff, entry.adr);
    }

    void visit(ast::BinOpNode& node) override {
        node.left->accept(*this);
        node.right->accept(*this);

        int opr_code = 0;
        using namespace lexer;
        if (node.op.type == TokenType::PLUS) opr_code = 2;
        else if (node.op.type == TokenType::MINUS) opr_code = 3;
        else if (node.op.type == TokenType::TIMES) opr_code = 4;
        else if (node.op.type == TokenType::IDIV || node.op.type == TokenType::RDIV) opr_code = 5;
        else if (node.op.type == TokenType::IMOD) opr_code = 6;
        else if (node.op.type == TokenType::EQL) opr_code = 7;
        else if (node.op.type == TokenType::NEQ) opr_code = 8;
        else if (node.op.type == TokenType::LSS) opr_code = 9;
        else if (node.op.type == TokenType::GEQ) opr_code = 10;
        else if (node.op.type == TokenType::GTR) opr_code = 11;
        else if (node.op.type == TokenType::LEQ) opr_code = 12;

        emit(OpCode::OPR, 0, opr_code);
    }

    void visit(ast::NumberNode& node) override {
        int value = std::stoi(node.val.lexeme);
        emit(OpCode::LIT, 0, value);
    }

    void visit(ast::IdentNode& node) override {
        const auto& entry = sym_table.getTabEntry(node.tab_index);
        int level_diff = current_level - entry.lev;
        emit(OpCode::LOD, level_diff, entry.adr);
    }

    void visit(ast::ConstDeclNode&) override {}
    void visit(ast::VarDeclNode&) override {}
    void visit(ast::TypeDeclNode&) override {}
    void visit(ast::ProcDeclNode&) override {}
    void visit(ast::FuncDeclNode&) override {}
    void visit(ast::UnaryOpNode& node) override {
        node.expr->accept(*this);
        if (node.op.type == lexer::TokenType::MINUS) {
            emit(OpCode::OPR, 0, 1);  // NEG
        }
    }
    void visit(ast::StringNode&) override {}
    void visit(ast::FuncCallNode&) override {}
    void visit(ast::ArrayAccessNode& node) override {
        int level_diff = 0;
        int base_adr = 0;
        if (!emitArrayRelativeIndex(node, level_diff, base_adr)) {
            return;
        }
        emit(OpCode::LDX, level_diff, base_adr);
    }
    void visit(ast::RecordAccessNode&) override {}
    void visit(ast::RepeatNode&) override {}
    void visit(ast::ForNode&) override {}

    void visit(ast::SimpleTypeSpecNode&) override {}
    void visit(ast::SubrangeTypeSpecNode&) override {}
    void visit(ast::ArrayTypeSpecNode&) override {}
    void visit(ast::RecordTypeSpecNode&) override {}
    void visit(ast::EnumTypeSpecNode&) override {}
};