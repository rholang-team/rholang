#pragma once

#include <unordered_map>

#include "compiler/ir/function.hpp"
#include "compiler/ir/value.hpp"

namespace ir {
class Module {
public:
    using Signatures =
        std::unordered_map<std::string, std::unique_ptr<FunctionSignature>>;
    using Globals = std::unordered_map<std::string, std::shared_ptr<GlobalPtr>>;
    using Functions =
        std::unordered_map<std::string, std::unique_ptr<Function>>;

private:
    std::unordered_map<std::string, std::vector<bool>> structMaps_;
    Signatures signatures_;
    Globals globals_;
    Functions functions_;

public:
    FunctionSignature* addSignature(std::unique_ptr<FunctionSignature> fn);
    Function* addFunction(std::unique_ptr<Function> fn);
    std::shared_ptr<GlobalPtr> addGlobal(std::string name,
                                         std::shared_ptr<GlobalPtr> global);

    std::unordered_map<std::string, std::vector<bool>> structMaps() const {
        return structMaps_;
    }

    template <typename T>
    void addStructMap(T&& n, std::vector<bool> m) {
        structMaps_.emplace(std::forward<T>(n), std::move(m));
    }

    Signatures& signatures() & {
        return signatures_;
    }
    const Signatures& signatures() const& {
        return signatures_;
    }

    Globals& globals() & {
        return globals_;
    }
    const Globals& globals() const& {
        return globals_;
    }

    Functions& functions() & {
        return functions_;
    }
    const Functions& functions() const& {
        return functions_;
    }

    bool operator==(const Module& that) const;
};
}  // namespace ir
