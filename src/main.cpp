#include "builtin.h"
#include "completion.h"
#include "path.h"
#include "shell.h"

#include <cstdlib>
#include <iostream>
#include <readline/readline.h>
#include <string>

using namespace std;

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

  while (true) {
    char *line = readline(constants::PROMPT);

    if (!line) {
      break; // EOF (Ctrl+D)
    }

    string input(line);
    free(line);

    if (input.empty()) {
      continue;
    }

    auto parsed = shell::parse(input);
    if (parsed.cmd.empty()) {
      continue;
    }

    [[maybe_unused]] int exit_code = shell::execute(parsed);
  }
}

