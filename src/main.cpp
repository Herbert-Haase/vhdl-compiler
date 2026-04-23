#include <fstream>
#include <iostream>

#include "antlr4-runtime.h"
#include "VHDLLexer.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file.vhd>\n";
        return 1;
    }

    std::ifstream input_stream(argv[1]);
    if (!input_stream) {
        std::cerr << "Failed to open file: " << argv[1] << '\n';
        return 1;
    }

    antlr4::ANTLRInputStream input(input_stream);
    VHDLLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    tokens.fill();

    if (lexer.getNumberOfSyntaxErrors() > 0)
        return 1;

    for (auto* token : tokens.getTokens()) {
        std::cout
            << "[@" << token->getTokenIndex() << "]"
            << " type=" << token->getType()
            << " line=" << token->getLine()
            << ":" << token->getCharPositionInLine()
            << " text=" << token->getText()
            << '\n';
    }

    return 0;
}
