#include <fstream>
#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <memory>
#include <sstream>

#include "antlr4-runtime.h"
#include "VHDLLexer.h"
#include "VHDLParser.h"

#include "ASTBuilder.hpp"
#include "ast.hpp"

// Helper to join vector of strings (for PortDecl and Signal names)
std::string joinNames(const std::vector<std::string>& names) {
    std::ostringstream oss;
    for (size_t i = 0; i < names.size(); ++i) {
        oss << names[i] << (i == names.size() - 1 ? "" : ", ");
    }
    return oss.str();
}

// Recursive helper for the ExprNode hierarchy
void printExprDot(ExprNode* expr, int& nodeId, int parentId) {
    if (!expr) return;
    int currentId = nodeId++;
    std::string label;

    if (auto* id = dynamic_cast<IdExpr*>(expr)) {
        label = "ID: " + id->name;
    } else if (auto* n = dynamic_cast<NotExpr*>(expr)) {
        label = "NOT";
        printExprDot(n->operand.get(), nodeId, currentId);
    } else if (auto* a = dynamic_cast<AndExpr*>(expr)) {
        label = "AND";
        for (auto& op : a->operands) printExprDot(op.get(), nodeId, currentId);
    } else if (auto* o = dynamic_cast<OrExpr*>(expr)) {
        label = "OR";
        for (auto& op : o->operands) printExprDot(op.get(), nodeId, currentId);
    }

    std::cout << "  n" << currentId << " [label=\"" << label << "\", shape=ellipse, style=dashed];\n";
    std::cout << "  n" << parentId << " -> n" << currentId << ";\n";
}

void printASTDot(ASTNode* node, int& nodeId, int parentId = -1) {
    if (!node) return;
    int currentId = nodeId++;
    std::string label;
    std::string color = "white";

    if (auto* start = dynamic_cast<StartRule*>(node)) {
        label = "StartRule";
        color = "lightblue";
        std::cout << "  n" << currentId << " [label=\"" << label << "\", fillcolor=\"" << color << "\", style=filled];\n";
        for (auto& lib : start->libs) printASTDot(lib.get(), nodeId, currentId);
        for (auto& use : start->uses) printASTDot(use.get(), nodeId, currentId);
        for (auto& ent : start->entities) printASTDot(ent.get(), nodeId, currentId);
        for (auto& arch : start->arches) printASTDot(arch.get(), nodeId, currentId);
        return;
    } 
    else if (auto* ent = dynamic_cast<Entity*>(node)) {
        label = "Entity: " + ent->name;
        color = "lightgreen";
        if (ent->port) printASTDot(ent->port.get(), nodeId, currentId);
    } 
    else if (auto* port = dynamic_cast<Port*>(node)) {
        label = "PORT";
        for (auto& sig : port->signals) printASTDot(sig.get(), nodeId, currentId);
    } 
    else if (auto* pd = dynamic_cast<PortDecl*>(node)) {
        label = (pd->in ? "[IN] " : "[OUT] ") + joinNames(pd->names) + " : " + pd->type;
        color = "lightyellow";
    }
    else if (auto* arch = dynamic_cast<Arch*>(node)) {
        label = "Arch: " + arch->name + " of " + arch->entity;
        color = "bisque";
        std::cout << "  n" << currentId << " [label=\"" << label << "\", fillcolor=\"" << color << "\", style=filled];\n";
        if (parentId != -1) std::cout << "  n" << parentId << " -> n" << currentId << ";\n";
        for (auto& sig : arch->signals) printASTDot(sig.get(), nodeId, currentId);
        for (auto& stmt : arch->statements) printASTDot(stmt.get(), nodeId, currentId);
        return;
    }
    else if (auto* sig = dynamic_cast<Signal*>(node)) {
        label = "Signal: " + joinNames(sig->names) + " : " + sig->type;
        color = "lightcyan";
    }
    else if (auto* stmt = dynamic_cast<Statement*>(node)) {
        label = "Assign: " + stmt->varName;
        color = "lavender";
        std::cout << "  n" << currentId << " [label=\"" << label << "\", fillcolor=\"" << color << "\", style=filled];\n";
        if (parentId != -1) std::cout << "  n" << parentId << " -> n" << currentId << ";\n";
        if (stmt->expr) printExprDot(stmt->expr.get(), nodeId, currentId);
        return;
    }
    else if (auto* lib = dynamic_cast<LibDecl*>(node)) {
        label = "Library: " + lib->name;
    }
    else if (auto* use = dynamic_cast<UseDecl*>(node)) {
        label = "Use: " + use->name;
    }

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
