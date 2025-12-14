#include "command.h"

#include <iostream>
#include <sstream>
#include <string>

namespace constants {
const char *PROMPT = "$ ";
const char *CMD_NOT_FOUND = ": command not found";
} // namespace constants

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  command::init();

  while (true) {
    std::cout << constants::PROMPT;

    std::string input;
    std::string command;
    std::vector<std::string> args;

    if (!std::getline(std::cin, input)) {
      break;
    }
    std::istringstream iss(input);
    iss >> command;
    if (command.empty()) {
      continue;
    }
    std::string arg;
    while (iss >> arg) {
      args.push_back(arg);
    }

    int code = command::execute(command, args);
    if (code == 127) {
      std::cout << command << constants::CMD_NOT_FOUND << std::endl;
    }
  }
}

