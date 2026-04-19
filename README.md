# vhdl-compiler

Minimal ANTLRv4 + C++ compiler project structure using CMake.

## Prerequisites

- CMake 3.20+
- C++17 compiler
- Java Runtime (for ANTLR code generation)

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/vhdl_compiler <path/to/file.vhd>
```

## Test

```bash
ctest --test-dir build --output-on-failure
```
