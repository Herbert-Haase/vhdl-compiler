// ASTBuilder.cpp

#include "ASTBuilder.hpp"
#include <memory>

void ASTBuilder::enterStartRule(VHDLParser::StartRuleContext *) {
  stack.push(new StartRule());
}

void ASTBuilder::exitStartRule(VHDLParser::StartRuleContext *) {
  root.reset(stack.top());
  stack.pop();
}

void ASTBuilder::exitLib_declaration(VHDLParser::Lib_declarationContext *ctx) {
  auto node = std::make_unique<LibDecl>();
  node->name = ctx->ID()->getText();
  static_cast<StartRule *>(stack.top())->libs.push_back(std::move(node));
}

void ASTBuilder::exitUse_declaration(VHDLParser::Use_declarationContext *ctx) {
  auto node = std::make_unique<UseDecl>();
  auto ids = ctx->ID();
  size_t id_size = ids.size();
  for (size_t i = 0; i < ids.size(); ++i) {
    node->name += ids[i]->getText();
    if (i < id_size - 1 || ctx->ALL())
      node->name += ".";
  }
  if (ctx->ALL() != nullptr) {
    node->name += "all";
    node->imports_all = true;
  } else {
    node->imports_all = false;
  }
  static_cast<StartRule *>(stack.top())->uses.push_back(std::move(node));
}

void ASTBuilder::enterEntity_unit(VHDLParser::Entity_unitContext *ctx) {
  stack.push(new Entity());
}

void ASTBuilder::exitEntity_unit(VHDLParser::Entity_unitContext *ctx) {
  auto node = static_cast<Entity *>(stack.top());
  stack.pop();
  node->name = ctx->ID(0)->getText();
  static_cast<StartRule *>(stack.top())
      ->entities.push_back(std::unique_ptr<Entity>(node));
}

void ASTBuilder::enterPort_obj(VHDLParser::Port_objContext *ctx) {}
