#ifndef COMMAND_H
#define COMMAND_H

#include <optional>
#include <string>
#include <vector>

namespace command {

struct Redirection {
  std::optional<std::string> input_file;  // <
  std::optional<std::string> output_file; // > or 1>
  bool append_output = false;             // true for >>, false for >
  std::optional<std::string> error_file;  // 2>
  bool append_error = false;              // true for 2>>, false for 2>
};

struct Input {
  std::string cmd;
  std::vector<std::string> args;
  Redirection redirection;
};

Input parse(const std::string &input);
void execute(const std::string &cmd, const std::vector<std::string> &args);

} // namespace command

#endif

