#include "command.h"
#include "builtin.h"
#include "redirection_guard.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
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

namespace command {

Input parse(const string &input) {
  Input result;
  string current;
  bool s_quote{false};
  bool d_quote{false};
  bool escaped{false};

  enum class NextToken {
    Normal,
    InputFile,
    OutputFile,
    ErrorFile
  };
  NextToken next_token = NextToken::Normal;

  auto process_token = [&](const string &token) {
    if (next_token == NextToken::InputFile) {
      result.redirection.input_file = token;
      next_token = NextToken::Normal;
    } else if (next_token == NextToken::OutputFile) {
      result.redirection.output_file = token;
      next_token = NextToken::Normal;
    } else if (next_token == NextToken::ErrorFile) {
      result.redirection.error_file = token;
      next_token = NextToken::Normal;
    } else if (token == "<") {
      next_token = NextToken::InputFile;
    } else if (token == ">" || token == "1>") {
      result.redirection.append_output = false;
      next_token = NextToken::OutputFile;
    } else if (token == ">>" || token == "1>>") {
      result.redirection.append_output = true;
      next_token = NextToken::OutputFile;
    } else if (token == "2>") {
      result.redirection.append_error = false;
      next_token = NextToken::ErrorFile;
    } else if (token == "2>>") {
      result.redirection.append_error = true;
      next_token = NextToken::ErrorFile;
    } else {
      if (result.cmd.empty()) {
        result.cmd = token;
      } else {
        result.args.push_back(token);
      }
    }
  };

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
      if (!current.empty()) {
        process_token(current);
        current.clear();
      }
    } else {
      current += c;
    }
  }

  if (!current.empty()) {
    process_token(current);
  }

  return result;
}

int execute(const string &cmd, const vector<string> &args, const Redirection &redir) {
  // Setup redirections (RAII - automatically restored when guard goes out of scope)
  RedirectionGuard guard(redir);

  if (builtin::is_builtin(cmd)) {
    return builtin::execute(cmd, args);
  }
  auto path = find_in_path(cmd);
  if (path) {
    return execute_external(cmd, *path, args);
  }
  cout << cmd << ": command not found" << endl;
  return 127;
}

} // namespace command
