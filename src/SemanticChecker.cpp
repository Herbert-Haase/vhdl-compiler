#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "SemanticChecker.hpp"
#include "ast.hpp"

void SemanticChecker::checkExpr(const ExprNode *node,
                                const std::unordered_set<std::string> &scope,
                                const std::string &archName) {
  if (auto *id = dynamic_cast<const IdExpr *>(node)) {
    if (!scope.count(id->name))
      errors.push_back("Undefined variable '" + id->name +
                       "' in architecture '" + archName + "'");
  } else if (auto *n = dynamic_cast<const NotExpr *>(node)) {
    checkExpr(n->operand.get(), scope, archName);
  } else if (auto *a = dynamic_cast<const AndExpr *>(node)) {
    for (auto &op : a->operands)
      checkExpr(op.get(), scope, archName);
  } else if (auto *o = dynamic_cast<const OrExpr *>(node)) {
    for (auto &op : o->operands)
      checkExpr(op.get(), scope, archName);
  }
}

void SemanticChecker::checkArchitectureEntities(const StartRule &root) {
  std::unordered_set<std::string> entityNames;
  for (auto &e : root.entities)
    entityNames.insert(e->name);

  for (auto &a : root.arches)
    if (entityNames.count(a->entity) < 1) {
      errors.push_back("Architecture '" + a->name +
                       "' references unknown entity '" + a->entity + "'");
    }
}

void SemanticChecker::checkArchitectureSignals(const Arch &arch,
                                               const Entity *entity) {
  std::unordered_set<std::string> scope;

  for (auto &s : arch.signals)
    for (auto &name : s->names)
      scope.insert(name);

  if (entity)
    for (auto &p : entity->port->signals)
      for (auto &name : p->names)
        scope.insert(name);

  for (auto &stmt : arch.statements) {
    checkExpr(stmt->expr.get(), scope, arch.name);
    if (!scope.count(stmt->varName))
      errors.push_back("Undefined variable '" + stmt->varName +
                       "' in architecture '" + arch.name + "'");
  }
}

bool SemanticChecker::check(const StartRule &root) {
  checkArchitectureEntities(root);

  std::unordered_map<std::string, Entity *> entityMap;
  for (auto &e : root.entities)
    entityMap[e->name] = e.get();

  for (auto &a : root.arches)
    checkArchitectureSignals(*a, entityMap[a->entity]);

  for (auto &err : errors)
    std::cerr << "Semantic error: " << err << '\n';

  return errors.empty();
}
