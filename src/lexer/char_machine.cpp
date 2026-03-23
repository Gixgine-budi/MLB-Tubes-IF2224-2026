#include "char_machine.hpp"

#include <cassert>
#include <stdexcept>

namespace {

char char_from_fgetc(int c) {
    assert(c != EOF);
    return static_cast<char>(static_cast<unsigned char>(static_cast<unsigned int>(c)));
}

}  // namespace

CharMachine::CharMachine(const std::string& filepath) {
    stream = std::fopen(filepath.c_str(), "r");
    if (stream == nullptr) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    const int c = std::fgetc(stream);
    if (c == EOF) {
        eof = true;
        current_char = '\0';
    } else {
        eof = false;
        current_char = char_from_fgetc(c);
    }
}

CharMachine::~CharMachine() {
    if (stream != nullptr) {
        std::fclose(stream);
        stream = nullptr;
    }
}

bool CharMachine::adv() {
    if (stream == nullptr) {
        throw std::runtime_error("Stream is not open");
    }
    if (eof) {
        return false;
    }
    const int c = std::fgetc(stream);
    if (c == EOF) {
        eof = true;
        current_char = '\0';
        return false;
    }
    current_char = char_from_fgetc(c);
    return true;
}

char CharMachine::curr() const {
    if (stream == nullptr) {
        throw std::runtime_error("Stream is not open");
    }
    if (eof) {
        throw std::runtime_error("No current character at end of file");
    }
    return current_char;
}

bool CharMachine::is_eof() const noexcept {
    return eof;
}
