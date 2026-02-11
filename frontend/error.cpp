#include "frontend/parse/error.hpp"

#include <cassert>
#include <sstream>

namespace frontend {
std::string Error::pretty() const {
    auto [b, e] = span;
    assert(e <= input.size());

    size_t lineStart = 0;
    for (size_t i = b; i > 0; --i) {
        if (input[i] == '\n') {
            lineStart = i + 1;
            break;
        }
    }

    size_t lineEnd = lineStart;
    for (size_t i = e; i < input.size(); ++i) {
        if (input[i] == '\n') {
            lineEnd = i;
            break;
        }
    }

    std::stringstream ss{};
    ss << msg << ":\n" << input.substr(lineStart, lineEnd - lineStart) << '\n';
    for (size_t i = lineStart; i < b; ++i) {
        ss << ' ';
    }
    for (size_t i = b; i <= e; ++i) {
        ss << '^';
    }
    return ss.str();
}
}  // namespace frontend
