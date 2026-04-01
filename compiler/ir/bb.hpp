#pragma once

#include <list>
#include <memory>

#include "compiler/ir/phi.hpp"

namespace ir {
class Instr;

class BasicBlock {
public:
    using Instrs = std::list<std::shared_ptr<Instr>>;
    // using Phis = std::vector<std::shared_ptr<PhiNode>>;

private:
    Instrs instrs_;
    bool hasTerminator_ = false;

    // Phis phis_;

public:
    void addInstr(std::shared_ptr<Instr> i);

    // void addPhi(std::shared_ptr<PhiNode> phi);

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

    // Phis::iterator phisBegin() {
    //     return phis_.begin();
    // }
    // Phis::const_iterator phisBegin() const {
    //     return phis_.begin();
    // }
    // Phis::const_iterator phisCbegin() const {
    //     return phis_.cbegin();
    // }

    // Phis::iterator phisEnd() {
    //     return phis_.end();
    // }
    // Phis::const_iterator phisEnd() const {
    //     return phis_.end();
    // }
    // Phis::const_iterator phisCend() const {
    //     return phis_.cend();
    // }
};
}  // namespace ir
