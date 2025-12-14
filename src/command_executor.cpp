#include "command_executor.h"

#include <cstdlib>
#include <iostream>
#include <string>

CommandExecutor::CommandExecutor() { this->registerBuiltinCommands(); }

int CommandExecutor::executeCommand(const std::string &command, const std::vector<std::string> &args) {
  if (this->builtins.count(command) == 0) {
    return 127;
  }
  return this->builtins[command](args);
}

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

void CommandExecutor::registerBuiltinCommands() {
  builtins["exit"] = builtin_exit;
  builtins["echo"] = builtin_echo;
}

