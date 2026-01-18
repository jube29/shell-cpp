#ifndef EXECUTION_H
#define EXECUTION_H

#include "command.h"

#include <functional>
#include <string>
#include <vector>

using namespace std;
using namespace command;

namespace exe {

int execute_external(const string &cmd, const string &path, const vector<string> &args);
int execute(const ParsedCommand &parsed);
void execute_pipeline(const vector<ParsedCommand> &cmds, const function<int(const ParsedCommand &)> &executor);
} // namespace exe

#endif

