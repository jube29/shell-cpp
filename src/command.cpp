#include "command.h"
#include "builtin.h"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

namespace {

optional<string> find_in_path(const string &cmd) {
  const char *path_env = getenv("PATH");
  if (!path_env)
    return nullopt;

  istringstream ss(path_env);
  string dir;
  while (getline(ss, dir, ':')) {
    string full_path = dir + "/" + cmd;
    if (access(full_path.c_str(), X_OK) == 0) {
      return full_path;
    }
  }
  return nullopt;
}

int execute_external(const string &cmd, const string &path, const vector<string> &args) {
  pid_t pid = fork();
  if (pid == 0) {
    vector<char *> argv;
    argv.push_back(const_cast<char *>(cmd.c_str()));
    for (const auto &arg : args) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);
    execv(path.c_str(), argv.data());
    exit(127);
  } else if (pid > 0) {
    int status;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
  }
  return 127;
}

} // namespace

namespace command {

Input parse(const string &input) {
  Input result;
  string current;
  bool s_quote{false};
  bool d_quote{false};
  bool escaped{false};

  for (size_t i = 0; i < input.size(); i++) {
    char c = input[i];
    if (escaped) {
      current += c;
      escaped = false;
    } else if (c == '\\' && !s_quote) {
      if (d_quote) {
        // In double quotes, backslash only escapes: $ ` " \ newline
        if (i + 1 < input.size()) {
          char next = input[i + 1];
          if (next == '$' || next == '`' || next == '"' || next == '\\' || next == '\n') {
            escaped = true;
          } else {
            current += c; // Keep the backslash literally
          }
        } else {
          current += c; // Trailing backslash, keep it
        }
      } else {
        escaped = true;
      }
    } else if (c == '\"' && !s_quote) {
      d_quote = !d_quote;
    } else if (c == '\'' && !d_quote) {
      s_quote = !s_quote;
    } else if (c == ' ' && !s_quote && !d_quote) {
      if (result.cmd.empty()) {
        result.cmd = current;
      } else if (!current.empty()) {
        result.args.push_back(current);
      }
      current.clear();
    } else {
      current += c;
    }
  }

  if (!current.empty()) {
    if (result.cmd.empty()) {
      result.cmd = current;
    } else {
      result.args.push_back(current);
    }
  }

  return result;
}

void execute(const string &cmd, const vector<string> &args) {
  if (builtin::is_builtin(cmd)) {
    builtin::execute(cmd, args);
    return;
  }
  auto path = find_in_path(cmd);
  if (path) {
    execute_external(cmd, *path, args);
    return;
  }
  cout << cmd << ": command not found" << endl;
}

} // namespace command
