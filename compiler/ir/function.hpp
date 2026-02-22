#pragma once

#include <list>

#include "compiler/ir/bb.hpp"
#include "compiler/ir/type.hpp"

namespace ir {
class Function {
    std::string name_;
    FunctionType* type_;
    std::list<std::unique_ptr<BasicBlock>> bbs_;

public:
    Function(std::string name, FunctionType* fntype)
        : name_{std::move(name)}, type_{fntype} {}

    std::string_view name() const {
        return name_;
    }
    FunctionType* type() const {
        return type_;
    }
    auto& bbs() {
        return bbs_;
    }
    const auto& bbs() const {
        return bbs_;
    }
};
}  // namespace ir
