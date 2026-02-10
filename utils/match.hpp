#pragma once

namespace utils {
template <typename T, typename U>
bool isa(const U* p) {
    return nullptr != dynamic_cast<const T*>(p);
}
}  // namespace utils
