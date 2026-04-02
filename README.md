# Arion Lexer (MLB-Tubes-IF2224-2026)

Arion is a Pascal-like programming language. This repository provides a modular, hand-written lexical analyzer (lexer) for Arion, implemented in C++.

The lexer processes source text one character at a time and classifies lexemes using a Deterministic Finite Automaton (DFA). It recognizes a comprehensive set of tokens including literals, operators, punctuation, keywords, identifiers, and comments. The implementation relies entirely on a custom state machine rather than auto-generated code.

This codebase includes the core C++ lexer components (`CharMachine`, `Lexer`, `Token`, states, alphabet) along with a command-line driver that tokenizes a given source file and outputs the resulting tokens.

## Contributors

Group K01 - MLB

|   NIM    |            Name            |       Role       |
| -------- | -------------------------- | ---------------- |
| 13524009 | Mikhael Benrael Tampubolon | Document writing |
| 13524011 | Muhammad Iqbal Raihan      | DFA graph design |
| 13524025 | Moh Hafizh Irham Perdana   | Document writing |
| 13524099 | Muhammad Akmal             | Implementations  |

## Requirements

- C++17 or newer
- CMake 3.16 or newer
- A compatible C++ compiler (GCC/Clang on Linux/macOS, MSVC or MinGW on Windows)

## Installation and usage

### Build

To compile the project from the repository root, run:

```
cmake -S . -B build
cmake --build build
```

### Run

The driver program expects exactly one argument: the path to an Arion source file (plain text in `.txt` format).

Linux or macOS:

```
./build/arion_interpreter path/to/source.txt
```

Windows (if using MSVC/Visual Studio generator):

```
build\Debug\arion_interpreter.exe path\to\source.txt
```

Windows (if using Ninja or MinGW generator):

```
build\arion_interpreter.exe path\to\source.txt
```

Tokens are printed to standard output, one per line. You can redirect the output to a file to save the token list:

Linux or macOS:

```
./build/arion_interpreter sample.txt > tokens.txt
```

Windows (PowerShell):

```
.\build\arion_interpreter.exe sample.txt | Out-File -Encoding ascii tokens.txt
```

### Error handling

If the argument count is incorrect, the program will print a brief usage message and terminate with a non-zero exit code.

## Repository layout

```text
.
├── bin/             # Dependency tracking files
├── build/           # CMake build output directory (generated)
├── doc/             # Documentation and design reports
├── src/             # C++ source code
│   ├── lexer/       # Core lexer modules (DFA states, tokens, char machine)
│   └── main.cpp     # Entry point and command-line driver
├── test/            # Sample inputs and expected outputs for testing
├── .clang-format    # Code formatting configuration
├── .gitignore       # Git ignore rules
├── CMakeLists.txt   # CMake build configuration
└── README.md        # Project documentation
```

## References

- Compiler Design - Lexical Analysis (TutorialsPoint)
- Hopcroft, Motwani, Ullman - Introduction to Automata Theory, Languages, and Computation, 3rd ed.
- Aho et al. - Compilers: Principles, Techniques, and Tools
- GNU Make manual - Introduction to Makefiles
- https://lcc.s3.amazonaws.com/book/pdf/06lexical-analysis.pdf
- https://symbolaris.com/course/Compilers/06-lex.pdf
- https://web.stanford.edu/class/archive/cs/cs143/cs143.1128/lectures/01/Slides01.pdf