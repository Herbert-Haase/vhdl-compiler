#include <fstream>
#include <iostream>

#include "antlr4-runtime.h"
#include "VHDLBaseListener.h"
#include "VHDLLexer.h"
#include "VHDLParser.h"

class EntityConsistencyListener final : public VHDLBaseListener {
public:
    bool has_name_mismatch = false;

    void enterEntity_declaration(VHDLParser::Entity_declarationContext* ctx) override {
        if (ctx->endName != nullptr && ctx->startName->getText() != ctx->endName->getText()) {
            has_name_mismatch = true;
        }
    }
};

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
    VHDLParser parser(&tokens);

    antlr4::tree::ParseTree* tree = parser.design_file();
    if (parser.getNumberOfSyntaxErrors() > 0) {
        return 1;
    }

    EntityConsistencyListener listener;
    antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    if (listener.has_name_mismatch) {
        std::cerr << "Entity end identifier must match its declaration name\n";
        return 1;
    }

    std::cout << "Parse successful\n";
    return 0;
}
