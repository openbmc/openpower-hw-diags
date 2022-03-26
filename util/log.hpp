#pragma once

// C++20 projects should use `lg2` over `log`, when possible. See the following
// for details on structured logging:
//   https://github.com/openbmc/phosphor-logging/blob/master/docs/structured-logging.md

#include <phosphor-logging/lg2.hpp>

// This allows for easier use of string literals (i.e. `"foo"s`). See the
// following for details:
//   https://en.cppreference.com/w/cpp/string/basic_string/operator%22%22s

#include <string>
using namespace std::string_literals;
