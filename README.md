# vhdl-compiler

Minimal ANTLRv4 + C++ compiler project structure using CMake.

## Prerequisites

- Bash
- C++17 compiler
- Java Runtime (for ANTLR code generation)
- antlr4-runtime

## Build

```bash
./compile.sh
```

## Run

```bash
./build/vhdl_lexer ./tests/fixtures/hello_word.vhd
```

## Example output

```bash
└── ❯ ./build/vhdl_lexer ./tests/fixtures/minimal_entity.vhd   
[@0] type=4 line=1:0 text=entity
[@1] type=26 line=1:7 text=adder
[@2] type=5 line=1:13 text=is
[@3] type=7 line=2:0 text=end
[@4] type=4 line=2:4 text=entity
[@5] type=26 line=2:11 text=adder
[@6] type=23 line=2:16 text=;
[@7] type=18446744073709551615 line=3:0 text=<EOF>

└── ❯ ./build/vhdl_lexer ./tests/fixtures/mismatched_entity.vhd
[@0] type=4 line=1:0 text=entity
[@1] type=26 line=1:7 text=adder
[@2] type=5 line=1:13 text=is
[@3] type=7 line=2:0 text=end
[@4] type=4 line=2:4 text=entity
[@5] type=26 line=2:11 text=subtractor
[@6] type=23 line=2:21 text=;
[@7] type=18446744073709551615 line=3:0 text=<EOF>
```
