#ifndef COMMAND_EXECUTOR_H
#define COMMAND_EXECUTOR_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

class CommandExecutor {
public:
  CommandExecutor();
  int executeCommand(const std::string &, const std::vector<std::string> &);

private:
  std::unordered_map<std::string, std::function<int(const std::vector<std::string> &)>> builtins;
  void registerBuiltinCommands();
};

#endif

