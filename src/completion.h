#ifndef COMPLETION_H
#define COMPLETION_H

#include <string>
#include <vector>

namespace completion {

void register_commands(const std::vector<std::string> &cmds);

} // namespace completion

#endif

