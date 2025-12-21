#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>

namespace command {

struct Input {
  std::string cmd;
  std::vector<std::string> args;
};

Input parse(const std::string &input);
void execute(const std::string &cmd, const std::vector<std::string> &args);

} // namespace command

#endif
