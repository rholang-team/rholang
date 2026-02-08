#pragma once

#include <ostream>

namespace frontend::pretty {
struct PrettyPrintable {
protected:
    void pad(std::ostream& os, unsigned depth) const;

public:
    virtual ~PrettyPrintable() = default;

    std::string pretty() const;

    virtual void pretty(std::ostream& os, unsigned depth = 0) const = 0;
};
}  // namespace frontend::pretty
