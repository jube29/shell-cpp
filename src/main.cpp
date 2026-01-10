#include "builtin.h"
#include "completion.h"
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
  vector<string> commands = builtin::get_builtin_names();
  completion::setup();
  completion::register_commands(commands);

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

