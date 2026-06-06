# Arion Interpreter

### MLB-Tubes-IF2224-2026

## Tentang Program

Arion Interpreter adalah sebuah proyek yang bertujuan untuk mengembangkan sebuah interpreter sederhana untuk bahasa pemrograman sendiri bernama Arion. Proyek ini mencakup implementasi lexer, parser, dan diagnoser untuk bahasa tersebut, serta integrasi ke dalam sebuah executable yang dapat dijalankan dari command line.

### Lexer

Lexer adalah komponen pertama dalam interpreter yang bertanggung jawab untuk menganalisis kode sumber karakter per karakter dan mengkonversinya menjadi token-token yang bermakna. 

**Metode Implementasi:**
- Menggunakan Deterministic Finite Automaton (DFA) untuk mengenali pola-pola leksikal
- Melakukan scanning satu pass melalui source code
- Mengidentifikasi token: keywords, identifiers, operators, literals, dan delimiters
- Menyimpan informasi line dan column untuk error reporting

### Parser

Parser adalah komponen kedua yang mengambil aliran token dari lexer dan membangun Parse Tree berdasarkan grammar bahasa Arion.

**Metode Implementasi:**
- Menggunakan teknik Recursive Descent Parsing
- Mengikuti context-free grammar untuk bahasa Arion
- Memvalidasi struktur sintaks program
- Menghasilkan parse tree yang merepresentasikan hierarki sintaks program
- Menangani operator precedence dan associativity dengan benar

### Semantic

Semantic adalah komponen ketiga yang melakukan analisis semantik pada parse tree untuk memastikan bahwa program tidak hanya benar secara sintaksis tetapi juga secara semantik.

**Metode Implementasi:**
- Melakukan Syntax Directed Translation (SDT) untuk menghitung atribut pada parse tree
- Membangun decorated Abstract Syntax Tree (AST) untuk representasi yang lebih abstrak dari program
- Menggunakan Visitor Pattern untuk traversal AST
- Membangun Symbol Table untuk menyimpan informasi tentang variabel, fungsi, dan tipe data
- Melakukan type checking untuk memastikan konsistensi tipe data dalam operasi dan assignment

### Error Handling (Diagnoser)

Diagnoser adalah komponen yang mendeteksi, mengklasifikasi, dan melaporkan berbagai kesalahan yang terjadi selama proses lexical analysis, parsing, dan semantic analysis.

**Metode Implementasi:**
- Menangkap error lexical (karakter invalid, literal format salah)
- Menangkap error syntax (token unexpected, struktur grammar salah)
- Menangkap error semantic (type mismatch, undefined variable)
- Menyimpan posisi error (line dan column) untuk diagnostic messages yang akurat
- Menggunakan error recovery untuk melanjutkan parsing dan menemukan error tambahan

## Kebutuhan Sistem

- C++17 atau lebih baru
- CMake 3.16 atau lebih baru
- Compiler C++ yang kompatibel (GCC/Clang di Linux/macOS, MSVC atau MinGW di Windows)

## Instalasi dan penggunaan

### Build

Untuk membangun proyek, jalankan perintah berikut di terminal dari direktori root proyek:
```sh
cmake -S . -B build
cmake --build build
```

### Menjalankan Program

* Langsung menjalankan executable. Hasil parsing ada di `path/to/source.txt.ptree`.
```sh
./arion path/to/source.txt --optional-flags
```

* Daftar flag:
  - `--lexer`: Hanya menjalankan lexer dan menyimpan token yang dihasilkan ke file `.token`
  - `--parser`: Menjalankan lexer dan parser, menyimpan parse tree yang dihasilkan ke file `.ptree`
  - `--semantic`: Menjalankan lexer, parser, dan semantic analyzer, menyimpan decorated AST ke file `.ast` dan symbol table ke file `.sym`
  - `--dump`: Digunakan bersamaan dengan flag lain untuk mencetak hasil terakhir ke terminal
  - `--dump-all`: Mencetak hasil dari semua tahap (lexer, parser, semantic) ke terminal

## Kontributor

**Kelompok MauLiburanBang**  
**K01 MLB**

### Milestone 1: Lexer

|   NIM    |           Nama             |      Peran       |
| -------- | -------------------------- | ---------------- |
| 13524009 | Mikhael Benrael Tampubolon | Penulisan Laporan |
| 13524011 | Muhammad Iqbal Raihan      | Desain DFA dan Diagram Transisi, Implementasi awal |
| 13524025 | Moh Hafizh Irham Perdana   | Penulisan Laporan |
| 13524099 | Muhammad Akmal             | Implementasi DFA, Implentasi seluruh Modul |

### Milestone 2: Parser

|   NIM    |           Nama             |      Peran       |
| -------- | -------------------------- | ---------------- |
| 13524009 | Mikhael Benrael Tampubolon | Penulisan Laporan, Parser Expression |
| 13524011 | Muhammad Iqbal Raihan      | Parser Function, Parser Statement |
| 13524025 | Moh Hafizh Irham Perdana   | Penulisan Laporan, Parser Header |
| 13524099 | Muhammad Akmal             | Implementasi Parser, Diagnoser, Integrasi ke Main |


### Milestone 3: Semantic

|   NIM    |           Nama             |      Peran       |
| -------- | -------------------------- | ---------------- |
| 13524009 | Mikhael Benrael Tampubolon | Penulisan Laporan, Semantic Analyzer |
| 13524011 | Muhammad Iqbal Raihan      | Semantic Analyzer dan SDT Builder |
| 13524025 | Moh Hafizh Irham Perdana   | Penulisan Laporan, SDT Builder |
| 13524099 | Muhammad Akmal             | Implementasi AST Node dan Visitor Pattern, Printer, Symbol Table, Integrasi ke Main |

*IF2224 Teori Bahasa Formal dan Automata*  
*Semester II Tahun Ajaran 2025/2026*

**Program Studi Teknik Informatika**  
**Sekolah Teknik Elektro dan Informatika**
**Institut Teknologi Bandung**
