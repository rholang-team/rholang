#pragma once

#include <vector>

#include "compiler/lir/bb.hpp"

namespace lir {
class Function {
public:
    using BBs = std::vector<std::unique_ptr<BasicBlock>>;

private:
    std::string label_;
    BBs bbs_;

public:
    template <typename S>
    explicit Function(S&& label) : label_{std::forward<S>(label)} {}

    std::string_view label() const {
        return label_;
    }

    void addBb(std::unique_ptr<BasicBlock> bb);

    size_t bbCount() const {
        return bbs_.size();
    }

    const BBs& bbs() const {
        return bbs_;
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
};

}  // namespace lir