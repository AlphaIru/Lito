# Forgive me that this is mostly vibe coded...

cmake_minimum_required(VERSION 3.20)

# Project info
project(Lito VERSION 0.1.0 LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# compilation databases
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Executable
add_executable(lito
    src/main.cpp
    src/lexer.cpp
)

# Header directories
target_include_directories(lito PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

enable_testing()

# Lexer tests.
add_executable(lito_lexer_tests
    tests/lexer_tests.cpp
    src/lexer.cpp
)

target_include_directories(lito_lexer_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_test(NAME LexerTests COMMAND lito_lexer_tests)

