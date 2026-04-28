#pragma once

#include <cstdint>

#include "compiler/lir/value.hpp"

namespace lir {
class VirtualRegister final : public Register {
public:
    using Id = size_t;

    Id id() const {
        return id_;
    }

private:
    Id id_;

    friend class VirtualRegisterFactory;

    VirtualRegister(Id id) : id_{id} {}
};

class VirtualRegisterFactory {
    VirtualRegister::Id counter_ = 0;

    VirtualRegister::Id nextId() {
        return counter_++;
    }

public:
    VirtualRegisterFactory() = default;

    void reset() {
        counter_ = 0;
    }

    VirtualRegister next() {
        return VirtualRegister{nextId()};
    }
    std::shared_ptr<VirtualRegister> nextShared() {
        return std::shared_ptr<VirtualRegister>(new VirtualRegister{nextId()});
    }
};

class PhysicalRegister final : public Register {
public:
    enum class Name : uint8_t {
        Rax,
        Rbx,
        Rcx,
        Rdx,
        Rsi,
        Rdi,
        Rbp,
        Rsp,
        R8,
        R9,
        R10,
        R11,
        R12,
        R13,
        R14,
        R15,
    };

private:
    Name name_;

public:
    PhysicalRegister(Name name) : name_{name} {}

    Name name() const {
        return name_;
    }

    static std::string_view nameToString(Name name);

    std::string_view toString() const {
        return nameToString(name_);
    }
};
}  // namespace lir