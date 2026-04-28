#include "compiler/lir/register.hpp"

namespace lir {
std::string_view PhysicalRegister::nameToString(Name name) {
    switch (name) {
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

}  // namespace lir
