#pragma once
#include "../ast/ast_visitor.hpp"
#include "../semantic/symbol_table.hpp"
#include "instruction.hpp"
#include <vector>

class CodeGenerator : public ast::ASTVisitor {
private:
    std::vector<Instruction> code;
    semantic::SymbolTable& sym_table; // FIX 1: Added semantic:: namespace
    int current_level;

    // Helper to add instructions to our code array
    int emit(OpCode op, int l, int a) {
        code.push_back({op, l, a});
        return code.size() - 1; // Return index for backpatching (Jumps)
    }

public:
    // FIX 1: Added semantic:: namespace
    CodeGenerator(semantic::SymbolTable& st) : sym_table(st), current_level(0) {}

    const std::vector<Instruction>& getCode() const { return code; }
    
    void printCode() const {
        for (size_t i = 0; i < code.size(); ++i) {
            std::cout << i << " " << code[i].getOpString() << " " 
                      << code[i].l << " " << code[i].a << "\n";
        }
    }

    void visit(ast::ProgramNode& node) override {
        int jmp_main = emit(OpCode::JMP, 0, 0);

        for (auto& decl : node.declarations) {
            decl->accept(*this);
        }

        code[jmp_main].a = code.size();

        // Initiate Memory for Main Block
        int main_vsze = sym_table.getBtabEntry(0).vsze; 
        emit(OpCode::INT, 0, 3 + main_vsze);

        node.compound_statement->accept(*this);
        emit(OpCode::RET, 0, 0);
    }

    void visit(ast::AssignNode& node) override {
        node.value->accept(*this);
        int tab_idx = node.target->tab_index;
        const auto& entry = sym_table.getTabEntry(tab_idx);
        int level_diff = current_level - entry.lev;
        emit(OpCode::STO, level_diff, entry.adr);
    }

    void visit(ast::BinOpNode& node) override {
        node.left->accept(*this);
        node.right->accept(*this);

        int opr_code = 0;
        
        // FIX 2: Compare node.op.type against lexer::TokenType enums
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

    void visit(ast::IntNode& node) override {
        emit(OpCode::LIT, 0, node.value); 
    }

    void visit(ast::IdentNode& node) override {
        const auto& entry = sym_table.getTabEntry(node.tab_index);
        int level_diff = current_level - entry.lev;
        emit(OpCode::LOD, level_diff, entry.adr); 
    }

    void visit(ast::IfNode& node) override {
        node.condition->accept(*this);
        int jpc_idx = emit(OpCode::JPC, 0, 0);
        node.then_branch->accept(*this);

        if (node.else_branch) {
            int jmp_idx = emit(OpCode::JMP, 0, 0);
            code[jpc_idx].a = code.size();
            node.else_branch->accept(*this);
            code[jmp_idx].a = code.size();
        } else {
            code[jpc_idx].a = code.size();
        }
    }

    void visit(ast::WhileNode& node) override {
        int start_idx = code.size(); 
        node.condition->accept(*this);
        int jpc_idx = emit(OpCode::JPC, 0, 0);
        node.body->accept(*this);
        emit(OpCode::JMP, 0, start_idx);
        code[jpc_idx].a = code.size();
    }

    void visit(ast::ProcCallNode& node) override {
        // FIX 3: Compare against the lexeme of the ID token
        if (node.id.lexeme == "writeln") {
            for (auto& arg : node.args) {
                arg->accept(*this); 
                emit(OpCode::OPR, 0, 13); // WRT 
            }
            emit(OpCode::OPR, 0, 14); // WRTLN 
            return;
        }

        const auto& entry = sym_table.getTabEntry(node.tab_index);
        for (auto& arg : node.args) {
            arg->accept(*this);
        }

        int level_diff = current_level - entry.lev;
        emit(OpCode::CAL, level_diff, entry.adr);
    }
};