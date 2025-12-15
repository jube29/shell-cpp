#include "command.h"

#include <iostream>
#include <string>

namespace constants {
const char *PROMPT = "$ ";
} // namespace constants

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  command::init();

  while (true) {
    std::cout << constants::PROMPT;

    std::string input;
    if (!std::getline(std::cin, input)) {
      break;
    }

    auto parsed = command::parse(input);
    if (parsed.cmd.empty()) {
      continue;
    }

    command::execute(parsed.cmd, parsed.args);
  }
}

