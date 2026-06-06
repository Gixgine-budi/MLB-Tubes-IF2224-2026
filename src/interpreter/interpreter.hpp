#pragma once
#include "../icg/instruction.hpp"
#include <vector>
#include <iostream>

class Interpreter {
private:
    std::vector<int> s; // The Stack
    int pc;             // Program Counter
    int b;              // Base Register
    int t;              // Top of Stack

    // Melacak Static Link berdasarkan lexical level difference
    int getBase(int l) {
        int b1 = b;
        while (l > 0) {
            b1 = s[b1]; 
            l--;
        }
        return b1;
    }

public:
    Interpreter() : s(10000, 0), pc(0), b(0), t(-1) {}

    void execute(const std::vector<Instruction>& code) {
        Instruction i;
        do {
            i = code[pc++];

            switch (i.op) {
                case OpCode::LIT:
                    s[++t] = i.a; 
                    break;
                case OpCode::LOD:
                    s[++t] = s[getBase(i.l) + i.a]; 
                    break;
                case OpCode::STO:
                    s[getBase(i.l) + i.a] = s[t--]; 
                    break;
                case OpCode::CAL:
                    s[t + 1] = getBase(i.l); // Static Link
                    s[t + 2] = b;            // Dynamic Link
                    s[t + 3] = pc;           // Return Address
                    b = t + 1;               
                    pc = i.a;                
                    break;
                case OpCode::INT:
                    t = t + i.a; 
                    break;
                case OpCode::JMP:
                    pc = i.a; 
                    break;
                case OpCode::JPC:
                    if (s[t--] == 0) { 
                        pc = i.a;
                    }
                    break;
                case OpCode::OPR:
                    switch (i.a) {
                        case 1: s[t] = -s[t]; break; // NEG
                        case 2: t--; s[t] = s[t] + s[t + 1]; break; // ADD
                        case 3: t--; s[t] = s[t] - s[t + 1]; break; // SUB
                        case 4: t--; s[t] = s[t] * s[t + 1]; break; // MUL
                        case 5: t--; s[t] = s[t] / s[t + 1]; break; // DIV
                        case 6: t--; s[t] = s[t] % s[t + 1]; break; // MOD
                        case 7: t--; s[t] = (s[t] == s[t + 1]) ? 1 : 0; break; // EQL
                        case 8: t--; s[t] = (s[t] != s[t + 1]) ? 1 : 0; break; // NEQ
                        case 9: t--; s[t] = (s[t] < s[t + 1]) ? 1 : 0; break;  // LSS
                        case 10: t--; s[t] = (s[t] >= s[t + 1]) ? 1 : 0; break; // GEQ
                        case 11: t--; s[t] = (s[t] > s[t + 1]) ? 1 : 0; break;  // GTR
                        case 12: t--; s[t] = (s[t] <= s[t + 1]) ? 1 : 0; break; // LEQ
                        case 13: std::cout << s[t--]; break; // WRT
                        case 14: std::cout << "\n"; break;   // WRTLN
                    }
                    break;
                case OpCode::RET:
                    t = b - 1;       
                    pc = s[b + 2];   
                    b = s[b + 1];    
                    break;
                case OpCode::LDX: {
                    int rel = s[t--];
                    s[++t] = s[getBase(i.l) + i.a + rel];
                    break;
                }
                case OpCode::STX: {
                    int val = s[t--];
                    int rel = s[t--];
                    s[getBase(i.l) + i.a + rel] = val;
                    break;
                }
            }
        } while (pc != 0);
    }
};