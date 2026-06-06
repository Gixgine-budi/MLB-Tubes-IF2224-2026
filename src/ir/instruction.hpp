#pragma once
#include <string>

enum class OpCode {
    LIT, LOD, STO, CAL, INT, JMP, JPC, OPR, RET,
    LDX,
    STX
};

struct Instruction {
    OpCode op;
    int l;
    int a;
    int aux;

    std::string getOpString() const {
        switch (op) {
            case OpCode::LIT: return "LIT";
            case OpCode::LOD: return "LOD";
            case OpCode::STO: return "STO";
            case OpCode::CAL: return "CAL";
            case OpCode::INT: return "INT";
            case OpCode::JMP: return "JMP";
            case OpCode::JPC: return "JPC";
            case OpCode::OPR: return "OPR";
            case OpCode::RET: return "RET";
            case OpCode::LDX: return "LDX";
            case OpCode::STX: return "STX";
            default: return "???";
        }
    }
};
