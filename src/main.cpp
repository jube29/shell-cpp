#include "builtin.h"
#include "completion.h"
#include "parsing.h"
#include "path.h"
#include "redirection_guard.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <readline/readline.h>
#include <sstream>
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

int execute(const parsing::ParsedCommand &parsed) {
  int exit_code;
  if (builtin::is_builtin(parsed.cmd)) {
    exit_code = builtin::execute(parsed.cmd, parsed.args);
  } else if (auto path = path::find_in_path(parsed.cmd)) {
    exit_code = execute_external(parsed.cmd, *path, parsed.args);
  } else {
    cout << parsed.cmd << ": command not found" << endl;
    exit_code = 127;
  }
  return exit_code;
}

void trim(string &str) {
  constexpr auto whitespace = " \t\n\r";
  auto start = str.find_first_not_of(whitespace);
  if (start == string::npos) {
    str.clear();
    return;
  }
  str.erase(0, start);
  str.erase(str.find_last_not_of(whitespace) + 1);
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

    istringstream ss(input);
    string sub_command;
    vector<parsing::ParsedCommand> sub_commands{};

    while (getline(ss, sub_command, '|')) {
      trim(sub_command);
      auto parsed = parsing::parse(sub_command);
      sub_commands.push_back(parsed);
    }

    if (sub_commands.empty() || sub_commands[0].cmd.empty())
      continue;

    const auto N = sub_commands.size();
    if (N == 1) {
      RedirectionGuard guard(sub_commands[0].redirection);
      [[maybe_unused]] auto code = execute(sub_commands[0]);
      continue;
    }
    using FileDescriptor = int;
    std::optional<FileDescriptor> read_from = std::nullopt;
    std::optional<FileDescriptor> write_to = std::nullopt;
    vector<pid_t> to_wait{};

    for (auto i{0uz}; i != N; i++) {
      FileDescriptor fd[2];
      if (i < N - 1) {
        pipe(fd);
        write_to = fd[1];
      } else {
        write_to = std::nullopt;
      }
      auto pid = fork();
      if (pid == -1) { // FORK ERROR
        close(fd[0]), close(fd[1]);
        cerr << "Fork error, exiting.";
      } else if (pid == 0) { // CHILD
        if (read_from) {
          dup2(*read_from, STDIN_FILENO);
        }
        if (write_to) {
          dup2(*write_to, STDOUT_FILENO);
        }
        RedirectionGuard guard(sub_commands[i].redirection);
        [[maybe_unused]] auto code = execute(sub_commands[i]);
        if (read_from) {
          close(*read_from);
        }
        if (write_to) {
          close(*write_to);
        }
        exit(code);
      } else { // PARENT
        if (i < N - 1) {
          if (write_to) {
            close(*write_to);
          }
          if (read_from) {
            close(*read_from);
          }
          read_from = fd[0];
        }
        to_wait.push_back(pid);
      }
    }
    if (read_from) {
      close(*read_from);
    }
    if (to_wait.size()) {
      for (auto child_pid : to_wait) {
        int status;
        waitpid(child_pid, &status, 0);
      }
    }
  }
}

