// ast.hpp

#include <memory>
#include <string>
#include <vector>

struct ASTNode {
  virtual ~ASTNode() = default;
};

struct LibDecl : ASTNode {
  std::string name;
};

struct UseDecl : ASTNode {
  std::string name;
};

struct PortDecl : ASTNode {
  std::string name;
  std::string type;
  bool in;
};

struct Port : ASTNode {
  std::vector<PortDecl> signals;
};

struct Entity : ASTNode {
  std::string name;
  std::unique_ptr<Port> port;
};

struct Signal : ASTNode {
  std::string name;
  std::string type;
};

struct Statement : ASTNode {
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

