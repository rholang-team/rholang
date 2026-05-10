#pragma once

#include <list>
#include <memory>
#include <optional>

namespace lir {
struct Instr;
class JmpInstr;

class BasicBlock {
public:
    using Instrs = std::list<std::unique_ptr<Instr>>;

private:
    size_t idx_;
    Instrs instrs_;

public:
    ~BasicBlock();
    BasicBlock(size_t idx);
    
    void addInstr(std::unique_ptr<Instr> i);

    size_t idx() const {
        return idx_;
    }

    std::optional<const lir::JmpInstr*> findJmp() const;
    bool hasReturn() const;

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