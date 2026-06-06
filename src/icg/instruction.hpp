#pragma once
#include <string>

enum class OpCode {
    LIT, LOD, STO, CAL, INT, JMP, JPC, OPR, RET,
    LDX,  // load array element: pop rel_index, push s[base + rel_index]
    STX   // store array element: pop value, pop rel_index, s[base + rel_index] = value
};

struct Instruction {
    OpCode op;
    int l; // Lexical Level difference
    int a; // Address / Value / Operation ID

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