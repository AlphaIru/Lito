# Forgive me that this is mostly vibe coded...

cmake_minimum_required(VERSION 3.20)

# Project info
project(Lito VERSION 0.1.0 LANGUAGES C CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compilation databases
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# LLVM backend
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND
    "${LLVM_DEFINITIONS}"
)

# Executable
add_executable(lito
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/codegen.cpp
)

# Header directories
target_include_directories(lito PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(lito PRIVATE
    ${LLVM_INCLUDE_DIRS}
)

target_compile_options(lito PRIVATE
    ${LLVM_DEFINITIONS_LIST}
)

target_link_libraries(lito PRIVATE
    LLVM
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


# Parser tests
add_executable(lito_parser_tests
    tests/parser_tests.cpp
    src/parser.cpp
    src/lexer.cpp
)

target_include_directories(lito_parser_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_test(NAME ParserTests COMMAND lito_parser_tests)

# LLVM code generator tests
add_executable(lito_codegen_tests
    tests/codegen_tests.cpp
    src/codegen.cpp
    src/parser.cpp
    src/lexer.cpp
)

target_include_directories(lito_codegen_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LLVM_INCLUDE_DIRS}
)

target_compile_options(lito_codegen_tests PRIVATE
    ${LLVM_DEFINITIONS_LIST}
)

target_link_libraries(lito_codegen_tests PRIVATE
    LLVM
)

add_test(NAME LLVMCodegenTests COMMAND lito_codegen_tests)
