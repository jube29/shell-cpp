#ifndef COMPLETION_H
#define COMPLETION_H

#include <string>
#include <vector>

namespace completion {

void setup();
void register_commands(const std::vector<std::string> &cmds);
char **completer(const char *word, int start, int end);

} // namespace completion

#endif

