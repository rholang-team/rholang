#pragma once

#include <cstdint>
#include <string>

#include "compiler/lir/value.hpp"

namespace lir {
class VirtualRegister final : public Register {
    size_t id_;

public:
    VirtualRegister(size_t id) : id_{id} {}

    size_t id() const {
        return id_;
    }
};

class PhysicalRegister final : public Register, public AssemblableValue {
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

    std::string toAsm() const override {
        switch (name_) {
            case Name::Rax:
                return "rax";
            case Name::Rbx:
                return "rbx";
            case Name::Rcx:
                return "rcx";
            case Name::Rdx:
                return "rdx";
            case Name::Rsi:
                return "rsi";
            case Name::Rdi:
                return "rdi";
            case Name::Rbp:
                return "rbp";
            case Name::Rsp:
                return "rsp";
            case Name::R8:
                return "r8";
            case Name::R9:
                return "r9";
            case Name::R10:
                return "r10";
            case Name::R11:
                return "r11";
            case Name::R12:
                return "r12";
            case Name::R13:
                return "r13";
            case Name::R14:
                return "r14";
            case Name::R15:
                return "r15";
        }
    }
};
}  // namespace lir