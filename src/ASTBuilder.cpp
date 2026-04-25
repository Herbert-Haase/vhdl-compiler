// ASTBuilder.cpp

#include "ASTBuilder.hpp"
#include "ast.hpp"
#include <algorithm>
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
  for (size_t i = 0; i < id_size; ++i) {
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

void ASTBuilder::enterEntity_unit(VHDLParser::Entity_unitContext *) {
  stack.push(new Entity());
}

void ASTBuilder::exitEntity_unit(VHDLParser::Entity_unitContext *ctx) {
  auto node = static_cast<Entity *>(stack.top());
  stack.pop();
  node->name = ctx->ID(0)->getText();
  static_cast<StartRule *>(stack.top())
      ->entities.push_back(std::unique_ptr<Entity>(node));
}

void ASTBuilder::enterPort_obj(VHDLParser::Port_objContext *ctx) {
  stack.push(new Port);
}

void ASTBuilder::exitPort_obj(VHDLParser::Port_objContext *ctx) {
  auto node = static_cast<Port *>(stack.top());
  stack.pop();
  static_cast<Entity *>(stack.top())->port = std::unique_ptr<Port>(node);
}

void ASTBuilder::exitIn_out_signal(VHDLParser::In_out_signalContext *ctx) {
  auto node = std::make_unique<PortDecl>();
  for (auto *id : ctx->ID()) {
    node->names.push_back(id->getText());
  }
  node->type = ctx->STD_LOGIC()->getText();
  node->in = ctx->IN() != nullptr;
  static_cast<Port *>(stack.top())->signals.push_back(std::move(node));
}

void ASTBuilder::enterArch_unit(VHDLParser::Arch_unitContext *ctx) {
  stack.push(new Arch());
}

void ASTBuilder::exitArch_unit(VHDLParser::Arch_unitContext *ctx) {
  auto *node = static_cast<Arch *>(stack.top());
  stack.pop();
  node->name = ctx->ID(0)->getText();
  node->entity = ctx->ID(1)->getText();
  static_cast<StartRule *>(stack.top())
      ->arches.push_back(std::unique_ptr<Arch>(node));
}

void ASTBuilder::exitSignal_obj(VHDLParser::Signal_objContext *ctx) {
  auto node = std::make_unique<Signal>();
  for (auto id : ctx->ID()) {
    node->names.push_back(id->getText());
  }
  node->type = ctx->STD_LOGIC()->getText();
  static_cast<Arch *>(stack.top())->signals.push_back(std::move(node));
}

void ASTBuilder::exitPrimary(VHDLParser::PrimaryContext *ctx) {
  if (ctx->ID() != nullptr) {
    auto node = std::make_unique<IdExpr>();
    node->name = ctx->ID()->getText();
    expr_stack.push(std::move(node));
  }
}

void ASTBuilder::exitNotExpr(VHDLParser::NotExprContext *ctx) {
  if (ctx->NOT() != nullptr) {
    auto node = std::make_unique<NotExpr>();
    node->operand = std::move(expr_stack.top());
    expr_stack.pop();
    expr_stack.push(std::move(node));
  }
}

void ASTBuilder::exitAndExpr(VHDLParser::AndExprContext *ctx) {
  if (!ctx->AND().empty()) {
    auto node = std::make_unique<AndExpr>();
    for (size_t i = 0; i < ctx->notExpr().size(); i++) {
      node->operands.push_back(std::move(expr_stack.top()));
      expr_stack.pop();
    }
    std::reverse(node->operands.begin(), node->operands.end());
    expr_stack.push(std::move(node));
  }
}

void ASTBuilder::exitOrExpr(VHDLParser::OrExprContext *ctx) {
  if (!ctx->OR().empty()) {
    auto node = std::make_unique<OrExpr>();
    for (auto *op : ctx->andExpr()) {
      node->operands.push_back(std::move(expr_stack.top()));
      expr_stack.pop();
    }
    std::reverse(node->operands.begin(), node->operands.end());
    expr_stack.push(std::move(node));
  }
}

void ASTBuilder::exitStatement(VHDLParser::StatementContext *ctx) {
  auto node = std::make_unique<Statement>();
  node->varName = ctx->ID()->getText();
  node->expr = std::move(expr_stack.top());
  expr_stack.pop();
  static_cast<Arch *>(stack.top())->statements.push_back(std::move(node));
}
