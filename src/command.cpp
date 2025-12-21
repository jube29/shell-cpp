#include "command.h"

#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits.h>
#include <linux/limits.h>
#include <optional>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_map>

using namespace std;

namespace {
// Forward declarations
int builtin_exit(const vector<string> &args);
int builtin_echo(const vector<string> &args);
int builtin_type(const vector<string> &args);
int builtin_pwd(const vector<string> &args);
int builtin_cd(const vector<string> &args);

unordered_map<string, function<int(const vector<string> &)>> builtins = {
    {"exit", builtin_exit},
    {"echo", builtin_echo},
    {"type", builtin_type},
    {"pwd", builtin_pwd},
    {"cd", builtin_cd},
};

bool is_builtin(const string &name) { return builtins.count(name) > 0; }

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

int builtin_exit(const vector<string> &args) {
  int code = args.empty() ? 0 : stoi(args[0]);
  exit(code);
  return code;
}

int builtin_echo(const vector<string> &args) {
  for (size_t i = 0; i < args.size(); i++) {
    if (i > 0)
      cout << " ";
    cout << args[i];
  }
  cout << endl;
  return 0;
}

int builtin_type(const vector<string> &args) {
  int code = 0;
  for (size_t i = 0; i < args.size(); i++) {
    if (is_builtin(args[i])) {
      cout << args[i] << " is a shell builtin" << endl;
    } else if (auto path = find_in_path(args[i])) {
      cout << args[i] << " is " << *path << endl;
    } else {
      cout << args[i] << ": not found" << endl;
      code = 1;
    }
  }
  return code;
}

int builtin_pwd(const vector<string> &args) {
  char cwd[PATH_MAX];
  if (getcwd(cwd, PATH_MAX) != nullptr) {
    cout << cwd << endl;
    return 0;
  }
  cerr << "pwd: error getting current directory" << endl;
  return 1;
}

int builtin_cd(const vector<string> &args) {
  if (args.empty()) {
    return 0;
  }
  string path = args[0];
  if (path[0] == '~') {
    char *home = getenv("HOME");
    if (!home) {
      cerr << "cd: HOME not set" << endl;
      return 1;
    }
    path.replace(0, 1, home);
  }
  if (chdir(path.c_str()) != 0) {
    cerr << "cd: " << args[0] << ": " << strerror(errno) << endl;
    return 1;
  }
  return 0;
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
  if (is_builtin(cmd)) {
    builtins[cmd](args);
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

