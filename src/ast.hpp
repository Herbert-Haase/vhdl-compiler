// ast.hpp

#pragma once

#include <memory>
#include <string>
#include <vector>

struct ExprNode {
  virtual ~ExprNode() = default;
};

struct IdExpr : ExprNode {
  std::string name;
};

struct NotExpr : ExprNode {
  std::unique_ptr<ExprNode> operand;
};

struct AndExpr : ExprNode {
  std::vector<std::unique_ptr<ExprNode>> operands;
};

struct OrExpr : ExprNode {
  std::vector<std::unique_ptr<ExprNode>> operands;
};

struct ASTNode {
  virtual ~ASTNode() = default;
};

struct LibDecl : ASTNode {
  std::string name;
};

struct UseDecl : ASTNode {
  std::string name;
  bool imports_all;
};

struct PortDecl : ASTNode {
  std::vector<std::string> names;
  std::string type;
  bool in;
};

struct Port : ASTNode {
  std::vector<std::unique_ptr<PortDecl>> signals;
};

struct Entity : ASTNode {
  std::string name;
  std::unique_ptr<Port> port;
};

struct Signal : ASTNode {
  std::vector<std::string> names;
  std::string type;
};

struct Statement : ASTNode {
  std::string varName;
  std::unique_ptr<ExprNode> expr;
};

struct Arch : ASTNode {
  std::string name;
  std::string entity;
  std::vector<std::unique_ptr<Signal>> signals;
  std::vector<std::unique_ptr<Statement>> statements;
};

struct StartRule : ASTNode {
  std::vector<std::unique_ptr<LibDecl>> libs;
  std::vector<std::unique_ptr<UseDecl>> uses;
  std::vector<std::unique_ptr<Entity>> entities;
  std::vector<std::unique_ptr<Arch>> arches;
};
