#pragma once
#include <string>
#include <vector>
#include <iostream>

// 9 Base Instructions defined in the Specification
enum class OpCode {
    LIT, LOD, STO, CAL, INT, JMP, JPC, OPR, RET
};

// Represents a single line of Intermediate Code
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
            default: return "???";
        }
    }
};