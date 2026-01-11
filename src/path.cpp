#include "path.h"

#include <cstdlib>
#include <dirent.h>
#include <optional>
#include <sstream>
#include <string>
#include <unistd.h>
#include <unordered_set>

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

vector<string> get_all_executables() {
  unordered_set<string> executables;
  const char *path_env = getenv("PATH");
  if (!path_env)
    return {};

  istringstream ss(path_env);
  string dir;
  while (getline(ss, dir, ':')) {
    DIR *dirp = opendir(dir.c_str());
    if (!dirp)
      continue;

    struct dirent *entry;
    while ((entry = readdir(dirp)) != nullptr) {
      if (entry->d_name[0] == '.')
        continue;

      string full_path = dir + "/" + entry->d_name;
      if (access(full_path.c_str(), X_OK) == 0) {
        executables.insert(entry->d_name);
      }
    }
    closedir(dirp);
  }

  return vector<string>(executables.begin(), executables.end());
}

} // namespace path
