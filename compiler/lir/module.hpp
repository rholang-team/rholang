#pragma once

#include <unordered_map>
#include <vector>

#include "compiler/lir/function.hpp"

namespace lir {
class Module {
public:
    using Globals = std::unordered_map<std::string, WordType>;
    using Functions = std::vector<Function>;

private:
    std::unordered_map<std::string, std::vector<bool>> structMaps_;
    Globals globals_;
    Functions functions_;

public:
    ~Module();
    Module();
    Module(Module&&);

    const std::unordered_map<std::string, std::vector<bool>>& structMaps()
        const {
        return structMaps_;
    }

    template <typename T>
    void addStructMap(T&& n, std::vector<bool> m) {
        structMaps_.emplace(n, std::move(m));
    }

    template <typename T>
    void putStructMaps(T&& m) {
        structMaps_ = std::forward<T>(m);
    }

    void addFunction(Function fn);

    template <typename T>
    void addGlobal(T&& name, WordType w) {
        globals_.emplace(std::forward<T>(name), w);
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
