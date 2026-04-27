#pragma once

#include <unordered_map>
#include <vector>

#include "compiler/lir/function.hpp"

namespace lir {
class Module {
    using Globals = std::unordered_map<std::string, size_t>;
    using Functions = std::vector<Function>;

private:
    Globals globals_;
    Functions functions_;

public:
    ~Module();
    Module();
    Module(Module&&);

    void addFunction(Function fn);

    template <typename T>
    void addGlobal(T&& name, size_t size) {
        globals_.emplace(std::forward<T>(name), size);
    }

    Globals& globals() {
        return globals_;
    }
    const Globals& globals() const {
        return globals_;
    }

    Functions& functions() {
        return functions_;
    }
    const Functions& functions() const {
        return functions_;
    }
};
}  // namespace lir
