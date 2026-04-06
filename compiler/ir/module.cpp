#include "compiler/ir/module.hpp"

namespace {
template <typename K, typename V>
bool ptrHashmapEq(const std::unordered_map<K, V>& lhs,
                  const std::unordered_map<K, V>& rhs,
                  auto cmp) {
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (const auto& [k, v] : lhs) {
        auto it = rhs.find(k);
        if (it == rhs.end()) {
            return false;
        }

        if (!cmp(it->second, v)) {
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

std::shared_ptr<GlobalPtr> Module::addGlobal(
    std::string name,
    std::shared_ptr<GlobalPtr> global) {
    globals_.emplace(std::move(name), global);
    return global;
}

bool Module::operator==(const Module& that) const {
    return ptrHashmapEq(
               signatures_,
               that.signatures_,
               [](const auto& a, const auto& b) { return *a == *b; }) &&
           ptrHashmapEq(globals_,
                        that.globals_,
                        [](const std::shared_ptr<GlobalPtr>& a,
                           const std::shared_ptr<GlobalPtr>& b) {
                            return a->operator==(*b);
                        }) &&
           ptrHashmapEq(functions_,
                        that.functions_,
                        [](const auto& a, const auto& b) { return *a == *b; });
}
}  // namespace ir
