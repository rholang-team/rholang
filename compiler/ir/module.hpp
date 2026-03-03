#pragma once

#include <unordered_map>

#include "compiler/ir/function.hpp"
#include "compiler/ir/value.hpp"

namespace ir {
class Module {
    std::unordered_map<std::string, std::unique_ptr<FunctionSignature>>
        signatures_;
    std::unordered_map<std::string, std::shared_ptr<Value>> globals_;
    std::unordered_map<std::string, std::unique_ptr<Function>> functions_;

public:
    void addSignature(std::unique_ptr<FunctionSignature> fn);
    void addFunction(std::unique_ptr<Function> fn);

    std::unordered_map<std::string, std::unique_ptr<FunctionSignature>>&
    signatures() & {
        return signatures_;
    }
    const std::unordered_map<std::string, std::unique_ptr<FunctionSignature>>&
    signatures() const& {
        return signatures_;
    }

    std::unordered_map<std::string, std::shared_ptr<Value>>& globals() & {
        return globals_;
    }
    const std::unordered_map<std::string, std::shared_ptr<Value>>& globals()
        const& {
        return globals_;
    }

    std::unordered_map<std::string, std::unique_ptr<Function>>& functions() & {
        return functions_;
    }
    const std::unordered_map<std::string, std::unique_ptr<Function>>&
    functions() const& {
        return functions_;
    }
};
}  // namespace ir
