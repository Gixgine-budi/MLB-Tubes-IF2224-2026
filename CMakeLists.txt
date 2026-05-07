cmake_minimum_required(VERSION 3.16)

project(arionin VERSION 0.1.2 LANGUAGES CXX)

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

target_include_directories(arion_io PUBLIC src)
target_include_directories(arion_diag PUBLIC src)
target_include_directories(arion_lexer PUBLIC src)
target_include_directories(arion_parser PUBLIC src)

# 
target_include_directories(arion_io PUBLIC src)
target_include_directories(arion_diag PUBLIC src)
target_include_directories(arion_lexer PUBLIC src)
target_include_directories(arion_parser PUBLIC src)

target_link_libraries(arion_lexer PUBLIC arion_diag)
# 

add_executable(arionin src/main.cpp)

target_link_libraries(arion_diag PUBLIC arion_io)
target_link_libraries(arion_lexer PUBLIC arion_diag)
target_link_libraries(arion_parser PUBLIC arion_lexer)
target_link_libraries(arion_parser PUBLIC arion_diag)
target_link_libraries(arionin PRIVATE arion_parser)

if(MSVC)
  target_compile_options(arionin PRIVATE /W4)
else()
  target_compile_options(arionin PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(
  TARGETS arionin
  RUNTIME DESTINATION bin)
