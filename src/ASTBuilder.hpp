// ASTBuilder.hpp
#pragma once
#include "VHDLLexer.h"
#include "VHDLParser.h"
#include "antlr4-runtime.h"

#include "VHDLParserBaseListener.h"
#include "ast.hpp"
#include <stack>

class ASTBuilder : public VHDLParserBaseListener {
public:
  std::unique_ptr<ASTNode> root;
  std::stack<std::unique_ptr<ExprNode>> expr_stack;

  // private:
  std::stack<ASTNode *> stack;

  // entry point
  void enterStartRule(VHDLParser::StartRuleContext *) override;
  void exitStartRule(VHDLParser::StartRuleContext *) override;

  // libs — no enter needed, built entirely from ctx in exit
  void exitLib_declaration(VHDLParser::Lib_declarationContext *) override;
  void exitUse_declaration(VHDLParser::Use_declarationContext *) override;

  // entity
  void enterEntity_unit(VHDLParser::Entity_unitContext *) override;
  void exitEntity_unit(VHDLParser::Entity_unitContext *) override;

  // ports — accumulates multiple in_out_signals, needs stack
  void enterPort_obj(VHDLParser::Port_objContext *) override;
  void exitPort_obj(VHDLParser::Port_objContext *) override;

  void exitIn_out_signal(VHDLParser::In_out_signalContext *) override;

  // architecture
  void enterArch_unit(VHDLParser::Arch_unitContext *) override;
  void exitArch_unit(VHDLParser::Arch_unitContext *) override;

  void exitSignal_obj(VHDLParser::Signal_objContext *) override;

  // Statement
  void exitStatement(VHDLParser::StatementContext *) override;

  // Expression
  void exitPrimary(VHDLParser::PrimaryContext *) override;
  void exitNotExpr(VHDLParser::NotExprContext *) override;
  void exitAndExpr(VHDLParser::AndExprContext *) override;
  void exitOrExpr(VHDLParser::OrExprContext *) override;
};
