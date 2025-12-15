#include "command.h"

#include <cstdlib>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_map>

namespace {
std::unordered_map<std::string, std::function<int(const std::vector<std::string> &)>> builtins;

bool is_builtin(const std::string &name) { return builtins.count(name) > 0; }

std::optional<std::string> find_in_path(const std::string &cmd) {
  const char *path_env = std::getenv("PATH");
  if (!path_env)
    return std::nullopt;

  std::istringstream ss(path_env);
  std::string dir;
  while (std::getline(ss, dir, ':')) {
    std::string full_path = dir + "/" + cmd;
    if (access(full_path.c_str(), X_OK) == 0) {
      return full_path;
    }
  }
  return std::nullopt;
}

int builtin_exit(const std::vector<std::string> &args) {
  int code = args.empty() ? 0 : std::stoi(args[0]);
  std::exit(code);
  return code;
}

int builtin_echo(const std::vector<std::string> &args) {
  for (size_t i = 0; i < args.size(); i++) {
    if (i > 0)
      std::cout << " ";
    std::cout << args[i];
  }
  std::cout << std::endl;
  return 0;
}

int builtin_type(const std::vector<std::string> &args) {
  int code = 0;
  for (size_t i = 0; i < args.size(); i++) {
    if (is_builtin(args[i])) {
      std::cout << args[i] << " is a shell builtin" << std::endl;
    } else if (auto path = find_in_path(args[i])) {
      std::cout << args[i] << " is " << *path << std::endl;
    } else {
      std::cout << args[i] << ": not found" << std::endl;
      code = 1;
    }
  }
  return code;
}

int execute_external(const std::string &cmd, const std::string &path,
                     const std::vector<std::string> &args) {
  pid_t pid = fork();
  if (pid == 0) {
    std::vector<char *> argv;
    argv.push_back(const_cast<char *>(cmd.c_str()));
    for (const auto &arg : args) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);
    execv(path.c_str(), argv.data());
    std::exit(127);
  } else if (pid > 0) {
    int status;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
  }
  return 127;
}
} // namespace

namespace command {
void init() {
  builtins["exit"] = builtin_exit;
  builtins["echo"] = builtin_echo;
  builtins["type"] = builtin_type;
}

void execute(const std::string &cmd, const std::vector<std::string> &args) {
  if (is_builtin(cmd)) {
    builtins[cmd](args);
    return;
  }
  auto path = find_in_path(cmd);
  if (path) {
    execute_external(cmd, *path, args);
    return;
  }
  std::cout << cmd << ": command not found" << std::endl;
}
} // namespace command

