#include "ast.hpp"
#include <string>
#include <unordered_set>

class SemanticChecker {
public:
  bool check(const StartRule &root);

private:
  std::vector<std::string> errors;

  void checkArchitectureEntities(const StartRule &root);
  void checkArchitectureSignals(const Arch &arch, const Entity *entity);
  void checkExpr(const ExprNode *node,
                 const std::unordered_set<std::string> &scope,
                 const std::string &archName);
};
