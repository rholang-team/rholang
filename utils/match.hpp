#pragma once

#include <concepts>

namespace utils {
template <typename Derived, typename Base>
    requires std::derived_from<Derived, Base>
bool isa(const Base* p) {
    return nullptr != dynamic_cast<const Derived*>(p);
}

template <typename Derived, typename Base>
    requires std::derived_from<Derived, Base>
bool polymorphicEq(const Base* a, const Base* b) {
    const Derived* pa = dynamic_cast<const Derived*>(a);
    const Derived* pb = dynamic_cast<const Derived*>(b);
    return pa != nullptr && pb != nullptr && *pa == *pb;
}
}  // namespace utils
