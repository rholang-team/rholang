#include "compiler/lir/register.hpp"

#include "utils/match.hpp"

namespace lir {
bool StackPointer::operator==(const Value& that) const {
    return utils::isa<StackPointer>(&that);
}

bool VirtualRegister::operator==(const Value& that) const {
    auto p = dynamic_cast<const VirtualRegister*>(&that);
    return p != nullptr && id_ == p->id_;
}

bool PhysicalRegister::operator==(const Value& that) const {
    auto p = dynamic_cast<const PhysicalRegister*>(&that);
    return p != nullptr && name_ == p->name_;
}

std::string_view wordTypeToString(WordType w) {
    switch (w) {
        case WordType::Byte:
            return "byte";
        case WordType::Word:
            return "word";
        case WordType::Dword:
            return "dword";
        case WordType::Qword:
            return "qword";
    }
}

size_t wordTypeToSize(WordType w) {
    switch (w) {
        case WordType::Byte:
            return 1;
        case WordType::Word:
            return 2;
        case WordType::Dword:
            return 4;
        case WordType::Qword:
            return 8;
    }
}

std::string_view PhysicalRegister::nameToString(Name name, WordType w) {
    switch (name) {
        case Name::Rax: {
            switch (w) {
                case WordType::Byte:
                    return "al";
                case WordType::Word:
                    return "ax";
                case WordType::Dword:
                    return "eax";
                case WordType::Qword:
                    return "rax";
            }
        }
        // case Name::Rbx: {
        //     switch (w) {
        //         case WordType::Byte:
        //             return "bl";
        //         case WordType::Word:
        //             return "bx";
        //         case WordType::Dword:
        //             return "ebx";
        //         case WordType::Qword:
        //             return "rbx";
        //     }
        // }
        case Name::Rcx: {
            switch (w) {
                case WordType::Byte:
                    return "cl";
                case WordType::Word:
                    return "cx";
                case WordType::Dword:
                    return "ecx";
                case WordType::Qword:
                    return "rcx";
            }
        }
        case Name::Rdx: {
            switch (w) {
                case WordType::Byte:
                    return "dl";
                case WordType::Word:
                    return "dx";
                case WordType::Dword:
                    return "edx";
                case WordType::Qword:
                    return "rdx";
            }
        }
        case Name::Rsi: {
            switch (w) {
                case WordType::Byte:
                    return "sil";
                case WordType::Word:
                    return "si";
                case WordType::Dword:
                    return "esi";
                case WordType::Qword:
                    return "rsi";
            }
        }
        case Name::Rdi: {
            switch (w) {
                case WordType::Byte:
                    return "dil";
                case WordType::Word:
                    return "di";
                case WordType::Dword:
                    return "edi";
                case WordType::Qword:
                    return "rdi";
            }
        }
        case Name::R8: {
            switch (w) {
                case WordType::Byte:
                    return "r8b";
                case WordType::Word:
                    return "r8w";
                case WordType::Dword:
                    return "r8d";
                case WordType::Qword:
                    return "r8";
            }
        }
        case Name::R9: {
            switch (w) {
                case WordType::Byte:
                    return "r9b";
                case WordType::Word:
                    return "r9w";
                case WordType::Dword:
                    return "r9d";
                case WordType::Qword:
                    return "r9";
            }
        }
        case Name::R10: {
            switch (w) {
                case WordType::Byte:
                    return "r10b";
                case WordType::Word:
                    return "r10w";
                case WordType::Dword:
                    return "r10d";
                case WordType::Qword:
                    return "r10";
            }
        }
        case Name::R11: {
            switch (w) {
                case WordType::Byte:
                    return "r11b";
                case WordType::Word:
                    return "r11w";
                case WordType::Dword:
                    return "r11d";
                case WordType::Qword:
                    return "r11";
            }
        }
            // case Name::R12: {
            //     switch (w) {
            //         case WordType::Byte:
            //             return "r12b";
            //         case WordType::Word:
            //             return "r12w";
            //         case WordType::Dword:
            //             return "r12d";
            //         case WordType::Qword:
            //             return "r12";
            //     }
            // }
            // case Name::R13: {
            //     switch (w) {
            //         case WordType::Byte:
            //             return "r13b";
            //         case WordType::Word:
            //             return "r13w";
            //         case WordType::Dword:
            //             return "r13d";
            //         case WordType::Qword:
            //             return "r13";
            //     }
            // }
            // case Name::R14: {
            //     switch (w) {
            //         case WordType::Byte:
            //             return "r14b";
            //         case WordType::Word:
            //             return "r14w";
            //         case WordType::Dword:
            //             return "r14d";
            //         case WordType::Qword:
            //             return "r14";
            //     }
            // }
            // case Name::R15: {
            //     switch (w) {
            //         case WordType::Byte:
            //             return "r15b";
            //         case WordType::Word:
            //             return "r15w";
            //         case WordType::Dword:
            //             return "r15d";
            //         case WordType::Qword:
            //             return "r15";
            //     }
            // }
    }
}

}  // namespace lir
