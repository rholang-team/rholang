#include "compiler/lir/instr.hpp"

#include <string_view>

namespace lir {
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
}  // namespace lir
