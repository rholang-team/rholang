#pragma once

#include <memory>
#include <vector>

namespace lir {
struct Instr;

class BasicBlock {
public:
    using Instrs = std::vector<std::unique_ptr<Instr>>;

private:
    Instrs instrs_;

public:
    void addInstr(std::unique_ptr<Instr> i);

    Instrs::iterator begin() {
        return instrs_.begin();
    }
    Instrs::const_iterator begin() const {
        return instrs_.begin();
    }
    Instrs::const_iterator cbegin() const {
        return instrs_.cbegin();
    }

    Instrs::iterator end() {
        return instrs_.end();
    }
    Instrs::const_iterator end() const {
        return instrs_.end();
    }
    Instrs::const_iterator cend() const {
        return instrs_.cend();
    }
};

}  // namespace lir