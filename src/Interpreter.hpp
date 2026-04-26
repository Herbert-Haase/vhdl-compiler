#include "ast.hpp"
#include <unordered_map>

class Interpreter {
public:
  void output(const StartRule &root) const;
  int filloutArgs(size_t argc, char **argv);

private:
  std::unordered_map<std::string, bool> env;

  bool evalExpr(const ExprNode *node) const;
};
