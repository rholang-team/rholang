#pragma once

#include <unordered_map>

#include "compiler/ir/function.hpp"
#include "compiler/ir/value.hpp"

namespace ir {
class Module {
    std::unordered_map<std::string, std::shared_ptr<Value>> globals_;
    std::unordered_map<std::string, std::unique_ptr<Function>> functions_;

public:
    void addFunction(std::unique_ptr<Function> fn);
};
}  // namespace ir
