#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>

namespace command {
void init();
void execute(const std::string &cmd, const std::vector<std::string> &args);
} // namespace command

#endif
