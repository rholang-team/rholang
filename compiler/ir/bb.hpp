#pragma once

#include <list>
#include <memory>

namespace ir {
class Instr;

class BasicBlock {
public:
    using Instrs = std::list<std::shared_ptr<Instr>>;

private:
    Instrs instrs_;
    bool hasTerminator_ = false;

public:
    void addInstr(std::shared_ptr<Instr> i);

    bool hasTerminator() const {
        return hasTerminator_;
    }

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

    bool operator==(const BasicBlock& that) const;
};
}  // namespace ir
