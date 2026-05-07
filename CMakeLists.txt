cmake_minimum_required(VERSION 3.16)

project(arionin VERSION 0.1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(
  arion_lexer
  
  src/lexer/char_machine.cpp
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

target_include_directories(arion_lexer PUBLIC src)
target_include_directories(arion_parser PUBLIC src)

add_executable(arionin src/main.cpp)

target_link_libraries(arionin PRIVATE arion_lexer arion_parser)

if(MSVC)
  target_compile_options(arionin PRIVATE /W4)
else()
  target_compile_options(arionin PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(
  TARGETS arionin
  RUNTIME DESTINATION bin)
