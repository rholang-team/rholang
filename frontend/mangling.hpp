#pragma once

#include <string>
#include <string_view>

namespace frontend {
std::string mangleMethodName(std::string_view structName, std::string_view methodName);
}  // namespace frontend
