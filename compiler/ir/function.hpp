#pragma once

#include <list>

#include "compiler/ir/bb.hpp"
#include "compiler/ir/type.hpp"

namespace ir {
class FunctionSignature {
    std::string name_;
    FunctionType* type_;

public:
    template <typename S>
    FunctionSignature(S&& s, FunctionType* type)
        : name_{std::forward<S>(s)}, type_{type} {}

    const std::string& name() const;
    const FunctionType* type() const;
};

class Function {
    std::shared_ptr<FunctionSignature> signature_;
    std::list<std::unique_ptr<BasicBlock>> bbs_;

public:
    explicit Function(std::shared_ptr<FunctionSignature> signature)
        : signature_{signature} {}

    std::shared_ptr<FunctionSignature> signature() const {
        return signature_;
    }
    auto& bbs() {
        return bbs_;
    }
    const auto& bbs() const {
        return bbs_;
    }
};
}  // namespace ir
