#ifndef PATH_H
#define PATH_H

#include <optional>
#include <string>

namespace path {

std::optional<std::string> find_in_path(const std::string &cmd);

} // namespace path

#endif
