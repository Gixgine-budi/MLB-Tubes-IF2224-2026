cmake_minimum_required(VERSION 3.16)

project(arion VERSION 0.2.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(
  arion_io
  src/io/char_machine.cpp
)

add_library(
  arion_diag
  src/diagnoser/diagnoser.cpp
)

add_library(
  arion_lexer
  src/lexer/lexer.cpp
  src/lexer/token.cpp
  src/lexer/state.cpp
  src/lexer/alphabet.cpp
)

add_library(
  arion_parser
  src/parser/parse_node.cpp
  src/parser/parser.cpp
  src/parser/parse_header.cpp
  src/parser/parse_function.cpp
  src/parser/parse_statement.cpp
  src/parser/parse_expression.cpp
)

add_library(
  arion_semantic
  src/semantic/sdt_builder.cpp
  src/semantic/sdt_declaration.cpp
  src/semantic/sdt_statement.cpp
  src/semantic/sdt_expression.cpp
)

target_include_directories(arion_io PUBLIC src)
target_include_directories(arion_diag PUBLIC src)
target_include_directories(arion_lexer PUBLIC src)
target_include_directories(arion_parser PUBLIC src)
target_include_directories(arion_semantic PUBLIC src)

target_include_directories(arion_io PUBLIC src)
target_include_directories(arion_diag PUBLIC src)
target_include_directories(arion_lexer PUBLIC src)
target_include_directories(arion_parser PUBLIC src)
target_include_directories(arion_semantic PUBLIC src)

target_link_libraries(arion_lexer PUBLIC arion_diag)

add_executable(arion src/main.cpp)

target_link_libraries(arion_diag PUBLIC arion_io)
target_link_libraries(arion_lexer PUBLIC arion_diag)
target_link_libraries(arion_parser PUBLIC arion_lexer)
target_link_libraries(arion_parser PUBLIC arion_diag)
target_link_libraries(arion_semantic PUBLIC arion_parser)
target_link_libraries(arion PRIVATE arion_semantic)

if(MSVC)
  target_compile_options(arion PRIVATE /W4)
else()
  target_compile_options(arion PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(
  TARGETS arion
  RUNTIME DESTINATION bin)
