#ifndef PATH_H
#define PATH_H

#include <optional>
#include <string>
#include <vector>

namespace path {

std::optional<std::string> find_in_path(const std::string &cmd);
std::vector<std::string> get_all_executables();

} // namespace path

#endif
