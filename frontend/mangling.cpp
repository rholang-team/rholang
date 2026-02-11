#include "mangling.hpp"

#include <format>

namespace frontend {
std::string mangleMethodName(std::string_view structName, std::string_view methodName) {
    return std::format("_R{}{}{}{}", structName.size(), structName, methodName.size(), methodName);
}

}  // namespace frontend
