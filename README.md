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


# Protokoll
Die Aufgabe bestand darin, aus dem zuvor erstellten Parse-Tree einen Abstract Syntax Tree (AST) aufzubauen. Der AST soll nur semantisch relevante Informationen des VHDL-Quellcodes enthalten und als Grundlage für spätere Compiler-Phasen dienen.

2. Sprachwechsel zu C++
Ab der AST-Aufgabe (02b) wurden die Aufgaben in C++ umgesetzt. Alle vorherigen Aufgaben (Lexer, Parser, Parse-Tree-Visualisierung) wurden entsprechend auf C++ portiert.

Der Wechsel erforderte eine intensive Einarbeitung in:
    • Den ANTLR4 C++ Runtime-API (generierter Code vs. selbst geschriebener Code)
    • Das C++ Memory-Management mit std::unique_ptr für die Baumstruktur
    • Den strukturellen Ablauf: welche Klassen ANTLR generiert, welche manuell implementiert werden müssen

3. Grundlegendes Konzept des AST
3.1 AST vs. Parse-Tree
Die Bezeichnung „Baum“ wird der tatsächlichen Struktur nur bedingt gerecht. Im Unterschied zu den bisher bekannten Binärbäumen handelt es sich beim AST um eine tiefe Verschachtelung von Klasseninstanzen mit folgenden Eigenschaften:
    • Nicht eine einzige Klasse für alle Knoten, sondern annähernd so viele Klassen wie es verschiedene Knotentypen gibt
    • Jede Klasse repräsentiert genau eine grammatikalische Konstruktion der Sprache (Entity, Architecture, Signal, Statement, ...)
    • Non-Terminale ohne semantischen Eigenwert (z. B. Wrapper-Regeln wie start, units, libs) erhalten keine eigene AST-Klasse
    • Terminal-Tokens ohne inhaltliche Bedeutung (z. B. das Schlüsselwort LIBRARY als String) werden nicht als Member gespeichert

3.2 Multiplizität als zentrales Entwurfskriterium
Der größte strukturelle Unterschied zwischen den AST-Klassen liegt in ihrer Multiplizität, also der Anzahl der Kindknoten einer Kategorie:
    • Genau ein Kindknoten: std::unique_ptr<ChildType> — z. B. Entity enthält genau ein Port-Objekt
    • Mehrere Kindknoten gleichen Typs: std::vector<std::unique_ptr<ChildType>> — z. B. Architecture enthält mehrere Statements

4. Implementierung
4.1 Listener-/Walker-Pattern
Zum Aufbau des AST wurde das Listener-/Walker-Pattern von ANTLR4 verwendet. Der ParseTreeWalker traversiert den Parse-Tree und ruft für jeden Knoten zwei Callbacks auf:
    • enterX — Pre-Order: wird beim Betreten eines Knotens aufgerufen (links nach rechts)
    • exitX — Post-Order: wird beim Verlassen eines Knotens aufgerufen, wenn alle Kinder bereits verarbeitet sind

4.2 Push-Attach und Build-Attach
Zum Zusammenbauen des AST wurden zwei Methodiken eingesetzt:
    • Push-Attach (Stack-basiert): Knoten, die auf die Fertigstellung von Kindknoten warten müssen, werden beim Enter auf einen Stack gelegt. Beim Exit werden sie vom Stack geholt, finalisiert und an den neuen Stack-Top angehängt. Beispiel: Entity wartet auf das Port-Objekt.
    • Build-Attach (kontextbasiert): Blätter oder einfache Knoten, deren Daten vollständig aus dem ctx-Objekt ausgelesen werden können, werden direkt im exitX aufgebaut und sofort an den aktuellen Stack-Top angehängt, ohne selbst auf den Stack zu kommen. Beispiel: LibDecl, Signal.

4.3 Separater Stack für Expressions
Die Ausdrucks-Grammatik (expression, orExpr, andExpr, notExpr, primary) ist rekursiv definiert. Da dieselbe Stack-Logik hier zu Konflikten mit dem Haupt-Stack geführt hätte, wurde ein eigener expr_stack vom Typ std::stack<std::unique_ptr<ExprNode>> eingesetzt.

Das Prinzip: Jedes exitX der Ausdrucks-Ebene legt sein Ergebnis auf den expr_stack. Passthrough-Regeln (z. B. orExpr ohne OR-Token) tun nichts — der Kindknoten liegt bereits oben. Nur exitStatement entnimmt das fertige Ausdrucksobjekt vom expr_stack und hängt es an das Statement-Objekt.

Wichtig: Die Reihenfolge der Operanden auf dem Stack ist umgekehrt (LIFO). Bei mehrelementigen Operatoren (AND, OR) müssen die Operanden nach dem Einsammeln umgekehrt werden (std::reverse), um die ursprüngliche Quellcode-Reihenfolge zu erhalten.

5. AST-Klassenstruktur (Überblick)
    • StartRule — Wurzelknoten: enthält Libs, Uses, Entities, Architectures
    • LibDecl, UseDecl — Bibliotheks- und Use-Deklarationen
    • Entity — Entitätsdeklaration mit optionalem Port
    • Port, PortDecl — Port-Objekt und einzelne Port-Signale (IN/OUT)
    • Arch — Architektur mit Signalen und Statements
    • Signal — interne Signaldeklaration
    • Statement — Zuweisung mit Zielname und Ausdrucksbaum
    • ExprNode-Hierarchie: IdExpr, NotExpr, AndExpr, OrExpr
