// ASTBuilder.cpp

#include "ASTBuilder.hpp"
#include <memory>

void ASTBuilder::enterStartRule(VHDLParser::StartRuleContext *) {
  stack.push(new StartRule()); // not unique pointer?
}

void ASTBuilder::exitStartRule(VHDLParser : StartRuleContext *) {
  root.reset(stack.top());
  stack.pop();
}

void ASTBuilder::exitLib_declaration(VHDLParser::Lib_declarationContext *ctx) {
  auto node = std::make_unique<LibDecl>();
  node->name = ctx->ID->getText();
  static_cast<ASTNode *>(stack.top())->libs.push_back(std::move(node));
}

void ASTBuilder::exitUse_declaration(VHDLParser::Lib_declarationContext *ctx) {
    auto node = std::make_unique()
}
