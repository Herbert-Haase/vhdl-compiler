# vhdl-compiler

Minimal ANTLRv4 + C++ compiler project structure using CMake.

## Prerequisites

- Bash
- C++17 compiler
- Java Runtime (for ANTLR code generation)
- antlr4-runtime
- git
- curl
- Graphviz

## Build

```bash
./compile.sh
```

## Run

```bash
./build/vhdl_parser ./tests/fixtures/hello_word.vhd > tree.dot
dot -Tsvg tree.dot -o tree.svg
brave tree.svg
```

## Example output
![parser-tree](./tree.svg)
