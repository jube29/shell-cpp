#include "command.h"

#include <cstdlib>
#include <functional>
#include <iostream>
#include <unordered_map>

namespace {
std::unordered_map<std::string, std::function<int(const std::vector<std::string> &)>> builtins;

bool is_builtin(const std::string &name) { return builtins.count(name) > 0; }

int builtin_exit(const std::vector<std::string> &args) {
  int code = args.empty() ? 0 : std::stoi(args[0]);
  std::exit(code);
  return code;
}

int builtin_echo(const std::vector<std::string> &args) {
  for (size_t i = 0; i < args.size(); i++) {
    if (i > 0)
      std::cout << " ";
    std::cout << args[i];
  }
  std::cout << std::endl;
  return 0;
}

int builtin_type(const std::vector<std::string> &args) {
  int code = 0;
  for (size_t i = 0; i < args.size(); i++) {
    if (is_builtin(args[i])) {
      std::cout << args[i] << " is a shell builtin" << std::endl;
    } else {
      ;
      std::cout << args[i] << ": not found" << std::endl;
      code = 1;
    }
  }
  return code;
}
} // namespace

namespace command {
void init() {
  builtins["exit"] = builtin_exit;
  builtins["echo"] = builtin_echo;
  builtins["type"] = builtin_type;
}

int execute(const std::string &cmd, const std::vector<std::string> &args) {
  if (!is_builtin(cmd))
    return 127;
  return builtins[cmd](args);
}
} // namespace command

