#ifndef SHELL_H
#define SHELL_H

#include "redirection_guard.h"

#include <string>
#include <vector>

namespace shell {

struct ParsedCommand {
  std::string cmd;
  std::vector<std::string> args;
  Redirection redirection;
};

ParsedCommand parse(const std::string &input);
int execute(const ParsedCommand &cmd);

} // namespace shell

#endif

