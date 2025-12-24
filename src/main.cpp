#include "command.h"

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
  while (true) {
    cout << constants::PROMPT;

    string input;
    if (!getline(cin, input)) {
      break;
    }

    auto parsed = command::parse(input);
    if (parsed.cmd.empty()) {
      continue;
    }

    [[maybe_unused]] int exit_code = command::execute(parsed);
  }
}

