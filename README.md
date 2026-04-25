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


Protokoll:
```
Ich habe für die Aufgabe zu C++ gewechselt.
Hat viel Zeit gekosten sich einzuarbeiten, bzw. die Logik zuverstehen was von ANTLR generiert wird und was selber geschrieben werden muss. Welcher strukturelle Ablauf gewollt ist.
Es einen Baum zunennen wird der Struktur nicht gerecht, es ist eine tiefe Verschachtelung von Klassen Instanzen. Anders als bei den Binär-Bäumen die wir bis jetzt kannten, gibt es nicht nur eine Klasse für alle Nodes, sondern fast so viele Klassen wie Nodes. 
Der größte Unterschied zwischen den Klassen war deren Multiplizität, dass heißt ob die Parent Node nur eine Child-Node der gleichen Kategorie hat (std::unique_ptr<>) oder mehrere (std::vector<unique_ptr<>>) Nodes. Alle Nodes die keine wichtigen Daten beinhalten, also non-Terminale, erhalten keine Klasse die instanziert wird und alle Daten von Terminal-Nodes, die unwichtig sind, z.B.: LIB enthält den String "Library", werden nicht als Member gespeichert.
Ich habe einen Listener-/Walker-Pattern verwendet. Enter ist pre-Order-traversal und exit ist post-order-traversal. zum Zusammenbauen habe ich die Pop-Attach und Build-Attach Methodik verwendet.
```
