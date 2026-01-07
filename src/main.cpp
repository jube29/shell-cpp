#include "builtin.h"
#include "completion.h"
#include "shell.h"

#include <iostream>
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
  completion::register_commands(commands);
  while (true) {
    cout << constants::PROMPT;

    string input;
    if (!getline(cin, input)) {
      break;
    }

    auto parsed = shell::parse(input);
    if (parsed.cmd.empty()) {
      continue;
    }

    [[maybe_unused]] int exit_code = shell::execute(parsed);
  }
}

