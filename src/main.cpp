#include "constants.h"

#include <iostream>
#include <string>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while (true) {
    std::cout << ShellConstants::PROMPT;
    std::string command;
    std::getline(std::cin, command);
    if (command == ShellConstants::EXIT_CMD)
      break;
    std::cout << command << ShellConstants::CMD_NOT_FOUND << std::endl;
  }
}

