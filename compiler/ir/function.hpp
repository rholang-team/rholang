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

    const std::string& name() const {
        return name_;
    }
    FunctionType* type() const {
        return type_;
    }

    bool operator==(const FunctionSignature& that) const = default;
};

class Function {
public:
    using BBs = std::list<std::unique_ptr<BasicBlock>>;

private:
    FunctionSignature* signature_;
    BBs bbs_;

public:
    explicit Function(FunctionSignature* signature) : signature_{signature} {}

    void addBB(std::unique_ptr<BasicBlock> bb) {
        bbs_.push_back(std::move(bb));
    }

    const FunctionSignature* signature() const {
        return signature_;
    }

    BBs::iterator begin() {
        return bbs_.begin();
    }
    BBs::const_iterator begin() const {
        return bbs_.begin();
    }
    BBs::const_iterator cbegin() const {
        return bbs_.cbegin();
    }

    BBs::iterator end() {
        return bbs_.end();
    }
    BBs::const_iterator end() const {
        return bbs_.end();
    }
    BBs::const_iterator cend() const {
        return bbs_.cend();
    }

    bool operator==(const Function& that) const {
        return *signature_ == *that.signature_ &&
               std::ranges::equal(
                   bbs_,
                   that.bbs_,
                   [](const auto& a, const auto& b) { return *a == *b; });
    }
};
}  // namespace ir
