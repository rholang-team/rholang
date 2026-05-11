#pragma once

#include <array>
#include <format>
#include <vector>

#include "compiler/lir/bb.hpp"
#include "compiler/lir/register.hpp"

namespace lir {
class Function {
public:
    using BBs = std::vector<std::unique_ptr<BasicBlock>>;

    static constexpr std::array<lir::PhysicalRegister::Name, 6>
        argumentRegisters{
            lir::PhysicalRegister::Name::Rdi,
            lir::PhysicalRegister::Name::Rsi,
            lir::PhysicalRegister::Name::Rdx,
            lir::PhysicalRegister::Name::Rcx,
            lir::PhysicalRegister::Name::R8,
            lir::PhysicalRegister::Name::R9,
        };

private:
    std::string label_;
    size_t paramCount_;
    bool returnsValue_;
    std::vector<bool> frameMap_;
    BBs bbs_;

public:
    template <typename S>
    explicit Function(S&& label, size_t argCount, bool returnsValue)
        : label_{std::forward<S>(label)},
          paramCount_{argCount},
          returnsValue_{returnsValue} {}

    std::string_view label() const {
        return label_;
    }

    std::string frameMapName() const {
        return std::format("_Rframemap{}", label_);
    }

    bool returnsValue() const {
        return returnsValue_;
    }

    size_t paramCount() const {
        return paramCount_;
    }

    void setFrameMap(std::vector<bool> frameMap) {
        frameMap_ = std::move(frameMap);
    }

    const std::vector<bool>& frameMap() const {
        return frameMap_;
    }

    void addBb(std::unique_ptr<BasicBlock> bb);

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