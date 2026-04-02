cmake_minimum_required(VERSION 3.16)

project(arion_interpreter VERSION 0.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(arion_lexer
  src/lexer/char_machine.cpp
  src/lexer/lexer.cpp
  src/lexer/token.cpp
  src/lexer/state.cpp
  src/lexer/alphabet.cpp
  )

target_include_directories(arion_lexer PUBLIC src/lexer)

add_executable(arion_interpreter src/main.cpp)

target_link_libraries(arion_interpreter PRIVATE arion_lexer)

if(MSVC)
  target_compile_options(arion_interpreter PRIVATE /W4)
else()
  target_compile_options(arion_interpreter PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(
  TARGETS arion_interpreter
  RUNTIME DESTINATION bin)
