#include "builtin.h"
#include "completion.h"
#include "path.h"
#include "redirection_guard.h"
#include "shell.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <readline/readline.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

namespace {

int execute_external(const string &cmd, const string &path, const vector<string> &args) {
  pid_t pid = fork();
  if (pid == -1) {
    cerr << "fork failed: " << strerror(errno) << endl;
    return 127;
  } else if (pid == 0) {
    vector<char *> argv;
    argv.push_back(const_cast<char *>(cmd.c_str()));
    for (const auto &arg : args) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);
    execv(path.c_str(), argv.data());
    exit(127);
  } else {
    int status;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
  }
}

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

    RedirectionGuard guard(parsed.redirection);

    [[maybe_unused]] int exit_code;
    if (builtin::is_builtin(parsed.cmd)) {
      exit_code = builtin::execute(parsed.cmd, parsed.args);
    } else {
      auto path = path::find_in_path(parsed.cmd);
      if (path) {
        exit_code = execute_external(parsed.cmd, *path, parsed.args);
      } else {
        cout << parsed.cmd << ": command not found" << endl;
        exit_code = 127;
      }
    }
  }
}

