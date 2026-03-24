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
        FILE* stream;
        char current_char;
        bool eof;
    public:
        CharMachine(const std::string& filepath);
        ~CharMachine();

        CharMachine(const CharMachine&) = delete;
        CharMachine& operator=(const CharMachine&) = delete;

        /**
         * @brief Advance the character machine by one character.
         * 
         * @return bool            True if the character was read successfully, false if the end of the stream was reached.
         * @throws std::runtime_error If the stream is closed.
         */
        bool adv();

        /**
         * @brief End the character machine.
         * 
         * @note This will close the stream and set the EOF flag to true.
         */
        void end() noexcept;

        /**
         * @brief Get the current character.
         * 
         * @return char            The current character.
         * @throws std::runtime_error If the stream is closed or at EOF.
         */
        char curr() const;

        /**
         * @brief Check if the character machine is at the end of the stream.
         * 
         * @return bool            True if the end of the stream was reached, false otherwise.
         */
        bool is_eof() const noexcept;
};
