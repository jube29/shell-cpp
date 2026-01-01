#include "path.h"

#include <cstdlib>
#include <optional>
#include <sstream>
#include <string>
#include <unistd.h>

using namespace std;

namespace path {

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

} // namespace path
