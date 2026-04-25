#include <fstream>
#include <iostream>
#include <string>
#include <regex>

#include "antlr4-runtime.h"
#include "VHDLLexer.h"
#include "VHDLParser.h"

#include "ASTBuilder.hpp"
#include "ast.hpp"

#include <iostream>
#include <vector>
#include <memory>

void printASTDot(ASTNode* node, int& nodeId, int parentId = -1) {
    if (!node) return;

    int currentId = nodeId++;
    std::string label;
    std::string color = "white";

    // 1. Determine Label and Color based on Node Type
    if (auto* start = dynamic_cast<StartRule*>(node)) {
        label = "StartRule";
        color = "lightblue";
        std::cout << "  n" << currentId << " [label=\"" << label << "\", fillcolor=\"" << color << "\", style=filled];\n";
        if (parentId != -1) std::cout << "  n" << parentId << " -> n" << currentId << ";\n";

        // Traverse vectors
        for (auto& lib : start->libs) printASTDot(lib.get(), nodeId, currentId);
        for (auto& use : start->uses) printASTDot(use.get(), nodeId, currentId);
        for (auto& ent : start->entities) printASTDot(ent.get(), nodeId, currentId);
        for (auto& arch : start->arches) printASTDot(arch.get(), nodeId, currentId);
        return; // Children handled manually
    } 
    else if (auto* lib = dynamic_cast<LibDecl*>(node)) {
        label = "Lib: " + lib->name;
        color = "lightyellow";
    } 
    else if (auto* use = dynamic_cast<UseDecl*>(node)) {
        label = "Use: " + use->name + (use->imports_all ? " (ALL)" : "");
        color = "lightgrey";
    } 
    else if (auto* ent = dynamic_cast<Entity*>(node)) {
        label = "Entity: " + ent->name;
        color = "lightgreen";
        std::cout << "  n" << currentId << " [label=\"" << label << "\", fillcolor=\"" << color << "\", style=filled];\n";
        if (parentId != -1) std::cout << "  n" << parentId << " -> n" << currentId << ";\n";
        
        // Entity children
        if (ent->port) printASTDot(ent->port.get(), nodeId, currentId);
        return;
    }
    else if (auto* p = dynamic_cast<Port*>(node)) {
        label = "Port List";
        std::cout << "  n" << currentId << " [label=\"" << label << "\"];\n";
        if (parentId != -1) std::cout << "  n" << parentId << " -> n" << currentId << ";\n";
        
        for (const auto& sig : p->signals) {
            int sigId = nodeId++;
            std::string sigLabel = (sig.in ? "[IN] " : "[OUT] ") + sig.name + " : " + sig.type;
            std::cout << "  n" << sigId << " [label=\"" << sigLabel << "\", shape=note];\n";
            std::cout << "  n" << currentId << " -> n" << sigId << ";\n";
        }
        return;
    }

    // Default Printing for simple nodes
    std::cout << "  n" << currentId << " [label=\"" << label << "\", fillcolor=\"" << color << "\", style=filled];\n";
    if (parentId != -1) std::cout << "  n" << parentId << " -> n" << currentId << ";\n";
}

// Wrapper to initialize the DOT format
void printASTDotWrapper(ASTNode* root) {
    std::cout << "digraph AST {\n";
    std::cout << "  node [shape=box, fontname=\"Courier\"];\n";
    int nodeId = 0;
    printASTDot(root, nodeId);
    std::cout << "}\n";
}

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

    ASTBuilder builder;
    antlr4::tree::ParseTreeWalker::DEFAULT.walk(&builder, tree);

    if (builder.root){
    printASTDotWrapper(builder.root.get());
    }

    return 0;
}
