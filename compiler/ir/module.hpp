#pragma once

#include <unordered_map>

#include "compiler/ir/function.hpp"
#include "compiler/ir/value.hpp"

namespace ir {
class Module {
    std::unordered_map<std::string, std::shared_ptr<Value>> globals;
    std::unordered_map<std::string, Function> functions;
};
}  // namespace ir
