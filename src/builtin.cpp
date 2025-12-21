#include "builtin.h"

#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <linux/limits.h>
#include <optional>
#include <sstream>
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

bool is_builtin_internal(const string &name);

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

unordered_map<string, function<int(const vector<string> &)>> builtins = {
    {"exit", builtin_exit},
    {"echo", builtin_echo},
    {"type", builtin_type},
    {"pwd", builtin_pwd},
    {"cd", builtin_cd},
};

bool is_builtin_internal(const string &name) { return builtins.count(name) > 0; }

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
    if (is_builtin_internal(args[i])) {
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

} // namespace

namespace builtin {

bool is_builtin(const string &name) { return is_builtin_internal(name); }

int execute(const string &cmd, const vector<string> &args) {
  return builtins[cmd](args);
}

} // namespace builtin
