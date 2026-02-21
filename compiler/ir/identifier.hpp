#pragma once

#include <string>
#include <utility>

namespace ir {
struct Identifier {
    enum class Kind : char {
        Local,
        Global
    };

    Kind kind;
    std::string name;
};
}  // namespace ir

template <>
struct std::hash<ir::Identifier> {
    size_t operator()(const ir::Identifier& id) const {
        return std::hash<std::string>{}(id.name) << std::to_underlying(id.kind);
    }
};
