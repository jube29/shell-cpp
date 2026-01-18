#include "builtin.h"
#include "path.h"

#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <linux/limits.h>
#include <optional>
#include <readline/history.h>
#include <unistd.h>
#include <unordered_map>

using namespace std;

namespace {

int history_last_append_idx = 0;

// Forward declarations
int builtin_exit(const vector<string> &args);
int builtin_echo(const vector<string> &args);
int builtin_type(const vector<string> &args);
int builtin_pwd(const vector<string> &args);
int builtin_cd(const vector<string> &args);
int builtin_history(const vector<string> &args);

bool is_builtin_internal(const string &name);

unordered_map<string, function<int(const vector<string> &)>> builtins = {
    {"exit", builtin_exit}, {"echo", builtin_echo}, {"type", builtin_type},
    {"pwd", builtin_pwd},   {"cd", builtin_cd},     {"history", builtin_history}};

bool is_builtin_internal(const string &name) { return builtins.count(name) > 0; }

int builtin_exit(const vector<string> &args) {
  int code = 0;
  if (!args.empty()) {
    char *end;
    long val = strtol(args[0].c_str(), &end, 10);
    if (*end != '\0' || end == args[0].c_str()) {
      cerr << "exit: " << args[0] << ": numeric argument required" << endl;
      code = 2;
    } else {
      code = static_cast<int>(val & 0xFF); // Exit codes are modulo 256
    }
  }
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
    } else if (auto path = path::find_in_path(args[i])) {
      cout << args[i] << " is " << *path << endl;
    } else {
      cout << args[i] << ": not found" << endl;
      code = 1;
    }
  }
  return code;
}

int builtin_pwd([[maybe_unused]] const vector<string> &args) {
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

int builtin_history(const vector<string> &args) {
  HISTORY_STATE *state = history_get_history_state();
  if (!state) {
    return 1;
  }
  if (!args.empty() && args[0] == "-r") {
    if (args.size() < 2) {
      cerr << "history: " << args[0] << ": option requires an argument" << endl;
      return 1;
    }
    if (read_history(args[1].c_str()) != 0) {
      cerr << "history: " << args[1] << ": " << strerror(errno) << endl;
      return 1;
    }
    return 0;
  }
  if (!args.empty() && args[0] == "-w") {
    if (args.size() < 2) {
      cerr << "history: " << args[0] << ": option requires an argument" << endl;
      return 1;
    }
    if (write_history(args[1].c_str()) != 0) {
      cerr << "history: " << args[1] << ": " << strerror(errno) << endl;
      return 1;
    }
    return 0;
  }
  if (!args.empty() && args[0] == "-a") {
    if (args.size() < 2) {
      cerr << "history: " << args[0] << ": option requires an argument" << endl;
      return 1;
    }
    if (append_history(state->length - history_last_append_idx, args[1].c_str()) != 0) {
      cerr << "history: " << args[1] << ": " << strerror(errno) << endl;
      return 1;
    }
    history_last_append_idx = state->length;
    return 0;
  }
  optional<int> offset = nullopt;
  if (!args.empty()) {
    char *ptr;
    offset = (int)strtol(args[0].c_str(), &ptr, 10);
    if (*ptr != '\0') {
      cerr << "history: " << args[0] << ": invalid number" << endl;
      return 1;
    }
    if (offset < 0) {
      cerr << "history: " << args[0] << ": negative number" << endl;
      return 1;
    }
  }
  int start = offset ? max(state->length - *offset, 0) : 0;
  for (auto i = start; state->entries[i] != nullptr; ++i) {
    cout << i + 1 << "  " << state->entries[i]->line << endl;
  }
  free(state);
  return 0;
}

} // namespace

namespace builtin {

bool is_builtin(const string &name) { return is_builtin_internal(name); }

int execute(const string &cmd, const vector<string> &args) {
  auto it = builtins.find(cmd);
  if (it == builtins.end()) {
    cerr << "builtin::execute: '" << cmd << "' is not a builtin" << endl;
    return 127;
  }
  return it->second(args);
}

vector<string> get_builtin_names() {
  vector<string> names;
  names.reserve(builtins.size());
  for (const auto &pair : builtins) {
    names.push_back(pair.first);
  }
  return names;
}

} // namespace builtin

