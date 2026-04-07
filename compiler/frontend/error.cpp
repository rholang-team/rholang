#include "compiler/frontend/parse/error.hpp"

#include <cassert>
#include <sstream>

namespace frontend {
std::string Error::pretty() const {
    auto [b, e] = span;
    assert(e <= input.size());

    size_t firstLineStart = 0;
    for (size_t i = b; i > 0; --i) {
        if (input[i] == '\n') {
            firstLineStart = i + 1;
            break;
        }
    }

    size_t lastLineEnd = firstLineStart;
    for (size_t i = e; i < input.size(); ++i) {
        if (input[i] == '\n') {
            lastLineEnd = i;
            break;
        }
    }

    std::stringstream ss{};
    ss << msg << ":\n";

    size_t i = firstLineStart;
    while (i < lastLineEnd) {
        size_t newline = std::min(input.find('\n', i), input.size());
        ss << input.substr(i, newline - i) << '\n';

        bool sawNonspace = false;
        while (i < newline) {
            if (i < b)
                ss << ' ';
            if (b <= i && i <= e) {
                sawNonspace = !std::isspace(input[i]) || sawNonspace;

                if (sawNonspace)
                    ss << '^';
                else
                    ss << ' ';
            }
            ++i;
        }
        ss << '\n';

        i = newline + 1;
    }

    return ss.str();
}
}  // namespace frontend
