#ifndef PARSING_H
#define PARSING_H

#include "command.h"
#include <string>

namespace parsing {

command::ParsedCommand parse(const std::string &input);

} // namespace parsing

#endif

