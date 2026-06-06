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
        node.expr->accept(*this); // Push RHS ke stack

        int tab_idx = node.target->tab_index;
        const auto& entry = sym_table.getTabEntry(tab_idx);
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
    void visit(ast::UnaryOpNode&) override {}
    void visit(ast::StringNode&) override {}
    void visit(ast::FuncCallNode&) override {}
    void visit(ast::ArrayAccessNode&) override {}
    void visit(ast::RecordAccessNode&) override {}
    void visit(ast::RepeatNode&) override {}
    void visit(ast::ForNode&) override {}

    void visit(ast::SimpleTypeSpecNode&) override {}
    void visit(ast::SubrangeTypeSpecNode&) override {}
    void visit(ast::ArrayTypeSpecNode&) override {}
    void visit(ast::RecordTypeSpecNode&) override {}
    void visit(ast::EnumTypeSpecNode&) override {}
};