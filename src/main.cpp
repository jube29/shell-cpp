#include "builtin.h"
#include "command.h"
#include "completion.h"
#include "execution.h"
#include "parsing.h"
#include "path.h"
#include "redirection_guard.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <readline/history.h>
#include <readline/readline.h>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

namespace {

// namespace

void trim(string &str) {
  constexpr auto whitespace = " \t\n\r";
  auto start = str.find_first_not_of(whitespace);
  if (start == string::npos) {
    str.clear();
    return;
  }
  str.erase(0, start);
  str.erase(str.find_last_not_of(whitespace) + 1);
}

bool history_enabled = true;

} // namespace

namespace constants {
const char *PROMPT = "$ ";
} // namespace constants

int main() {
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;

  completion::setup();

  vector<string> commands = builtin::get_builtin_names();
  completion::register_commands(commands);

  vector<string> executables = path::get_all_executables();
  completion::register_commands(executables);

  if (char *history_file = getenv("HISTFILE"); history_file && read_history(history_file) == 0) {
    atexit([]() { write_history(getenv("HISTFILE")); });
  }

  while (true) {
    unique_ptr<char, decltype(&free)> line(readline(constants::PROMPT), free);

    if (!line) {
      break; // EOF (Ctrl+D)
    }

    if (history_enabled) {
      add_history(line.get());
    }

    string input(line.get());

    if (input.empty()) {
      continue;
    }

    istringstream ss(input);
    string sub_command;
    vector<ParsedCommand> sub_commands{};

    while (getline(ss, sub_command, '|')) {
      trim(sub_command);
      auto parsed = parsing::parse(sub_command);
      sub_commands.push_back(parsed);
    }

    if (sub_commands.empty() || sub_commands[0].cmd.empty())
      continue;

    const auto N = sub_commands.size();
    if (N == 1) {
      RedirectionGuard guard(sub_commands[0].redirection);
      [[maybe_unused]] auto code = exe::execute(sub_commands[0]);
    } else {
      exe::execute_pipeline(sub_commands, [&](const ParsedCommand &cmd) {
        RedirectionGuard guard(cmd.redirection);
        return exe::execute(cmd);
      });
    }
  }
}

