#ifndef BUILTIN_H
#define BUILTIN_H

#include <string>
#include <vector>

namespace builtin {

bool is_builtin(const std::string &name);
int execute(const std::string &cmd, const std::vector<std::string> &args);
std::vector<std::string> get_builtin_names();

} // namespace builtin

#endif
