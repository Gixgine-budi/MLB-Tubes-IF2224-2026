#pragma once
#include <cstdio>
#include <string>

/**
 * Buffered single-character reader over a FILE*.
 * After construction, the first input character is already loaded (or EOF is set for an empty file).
 * Use is_eof() before curr() when at end of file; curr() throws if called at EOF.
 */
class CharMachine {
    private:
        char current_char;
        bool eof;
    public:
        FILE* stream;

        CharMachine(const std::string& filepath);
        virtual ~CharMachine();

        /** Read the next character. Returns false at and after EOF (idempotent once EOF). */
        bool adv();

        /** Current character; throws if stream is closed or at EOF. */
        char curr() const;

        bool is_eof() const noexcept;
};
