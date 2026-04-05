#include "compiler/ir/module.hpp"

namespace {
template <typename K, typename V>
    requires std::equality_comparable<K> && requires(const V& v) {
        { *v } -> std::equality_comparable;
    }
bool ptrHashmapEq(const std::unordered_map<K, V>& lhs,
                  const std::unordered_map<K, V>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (const auto& [k, v] : lhs) {
        auto it = rhs.find(k);
        if (it == rhs.end()) {
            return false;
        }

        if (*it->second != *v) {
            return false;
        }
    }

    return true;
}
}  // namespace

namespace ir {
FunctionSignature* Module::addSignature(std::unique_ptr<FunctionSignature> fn) {
    FunctionSignature* res = fn.get();
    signatures_.emplace(fn->name(), std::move(fn));
    return res;
}

Function* Module::addFunction(std::unique_ptr<Function> fn) {
    Function* res = fn.get();
    functions_.emplace(fn->signature()->name(), std::move(fn));
    return res;
}

bool Module::operator==(const Module& that) const {
    return ptrHashmapEq(signatures_, that.signatures_) &&
           ptrHashmapEq(globals_, that.globals_) &&
           ptrHashmapEq(functions_, that.functions_);
}
}  // namespace ir
