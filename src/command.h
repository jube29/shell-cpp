#ifndef COMMAND_H
#define COMMAND_H

#include "redirection_guard.h"

#include <string>
#include <vector>

namespace command {

struct Input {
  std::string cmd;
  std::vector<std::string> args;
  Redirection redirection;
};

Input parse(const std::string &input);
int execute(const std::string &cmd, const std::vector<std::string> &args,
            const Redirection &redir);

} // namespace command

#endif

