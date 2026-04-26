#include <iostream>
#include <string>
#include <unordered_map>

#include "Interpreter.hpp"
#include "ast.hpp"

bool Interpreter::evalExpr(const ExprNode *node) const {
  if (auto *id = dynamic_cast<const IdExpr *>(node))
    return env.at(id->name);
  if (auto *n = dynamic_cast<const NotExpr *>(node))
    return !evalExpr(n->operand.get());
  if (auto *a = dynamic_cast<const AndExpr *>(node)) {
    for (auto &op : a->operands)
      if (!evalExpr(op.get()))
        return false;
    return true;
  }
  if (auto *o = dynamic_cast<const OrExpr *>(node)) {
    for (auto &op : o->operands)
      if (evalExpr(op.get()))
        return true;
    return false;
  }
  return false;
}

void Interpreter::output(const StartRule &root) const {
  std::cout << "Output:\n";
  for (auto &a : root.arches)
    for (auto &s : a->statements) {
      bool result = this->evalExpr(s->expr.get());
      std::string resultNum = result ? "1" : "0";
      std::cout << s->varName << " = " << resultNum << '\n';
    }
}

int Interpreter::filloutArgs(size_t argc, char **argv) {
  std::vector<std::string> args(argv, argv + argc);
  for (size_t i = 2; i < argc; ++i) {
    size_t pos;
    if ((pos = args[i].find('=')) == std::string::npos) {
      std::cerr << "'" << args[i] << "'"
                << " is not a proper assignment, use e.g a=1";
      return 1;
    }
    if (args[i].substr(pos + 1) == "1" || args[i].substr(pos + 1) == "true")
      this->env[args[i].substr(0, pos)] = true;
    else {
      if (args[i].substr(pos + 1) == "0" || args[i].substr(pos + 1) == "false")
        this->env[args[i].substr(0, pos)] = false;
      else {
        std::cerr << "'" << args[i] << "'"
                  << " is not a proper assignment, use e.g a=1";
        return 1;
      }
    }
  }
  return 0;
}
