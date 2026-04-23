#include <fstream>
#include <iostream>
#include <string>
#include <regex>

#include "antlr4-runtime.h"
#include "VHDLLexer.h"
#include "VHDLParser.h"

// Force all ANTLR diagnostics to stderr
class StderrErrorListener : public antlr4::BaseErrorListener {
public:
    void syntaxError(antlr4::Recognizer*, antlr4::Token*,
                     size_t line, size_t col,
                     const std::string& msg, std::exception_ptr) override {
        std::cerr << "line " << line << ":" << col << " " << msg << "\n";
    }
};

std::string escapeDot(const std::string& s) {
    std::string result = s;
    result = std::regex_replace(result, std::regex("\\\\"), "\\\\");
    result = std::regex_replace(result, std::regex("\""),   "\\\"");
    result = std::regex_replace(result, std::regex("\n"),   "\\n");
    return result;
}

void printDotTree(antlr4::tree::ParseTree* tree, VHDLParser& parser, int& nodeId, int parentId = -1) {
    int currentId = nodeId++;

    if (auto* leaf = dynamic_cast<antlr4::tree::TerminalNode*>(tree)) {
        auto* token = leaf->getSymbol();
        std::string label = std::string(parser.getVocabulary().getSymbolicName(token->getType()));
        if (label.empty()) label = token->getText();
        std::cout << "  n" << currentId << " [label=\"" << escapeDot(label) << "\"];\n";
        if (parentId != -1)
            std::cout << "  n" << parentId << " -> n" << currentId << ";\n";
    } else if (auto* rule = dynamic_cast<antlr4::RuleContext*>(tree)) {
        std::string label = parser.getRuleNames()[rule->getRuleIndex()];
        std::cout << "  n" << currentId << " [label=\"" << escapeDot(label) << "\"];\n";
        if (parentId != -1)
            std::cout << "  n" << parentId << " -> n" << currentId << ";\n";
        for (auto* child : rule->children)
            printDotTree(child, parser, nodeId, currentId);
    }
}

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

    StderrErrorListener errorListener;

    antlr4::ANTLRInputStream input(input_stream);
    VHDLLexer lexer(&input);
    lexer.removeErrorListeners();
    lexer.addErrorListener(&errorListener);

    antlr4::CommonTokenStream tokens(&lexer);
    VHDLParser parser(&tokens);
    parser.removeErrorListeners();
    parser.addErrorListener(&errorListener);

    antlr4::tree::ParseTree* tree = parser.startRule();

    if (parser.getNumberOfSyntaxErrors() > 0)
        return 1;

    std::cout << "digraph ParseTree {\n";
    std::cout << "  node [shape=box];\n";
    int nodeId = 0;
    printDotTree(tree, parser, nodeId);
    std::cout << "}\n";

    return 0;
}
