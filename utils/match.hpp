#pragma once

#include <concepts>
#include <memory>

namespace utils {
template <typename Derived, typename Base>
    requires std::derived_from<Derived, Base>
bool isa(const Base* x) {
    return nullptr != dynamic_cast<const Derived*>(x);
}

template <typename Derived, typename Base>
    requires std::derived_from<Derived, Base>
bool isa(const Base& x) {
    return nullptr != dynamic_cast<const Derived*>(std::addressof(x));
}

template <typename Base, typename Derived>
    requires std::derived_from<Derived, Base>
bool polymorphicEq(const Base* a, const Base* b) {
    const Derived* pa = dynamic_cast<const Derived*>(a);
    const Derived* pb = dynamic_cast<const Derived*>(b);
    return pa != nullptr && pb != nullptr && pa->Derived::operator==(*pb);
}

template <typename Base,
          typename DerivedFirst,
          typename DerivedSecond,
          typename... Derived>
bool polymorphicEq(const Base* a, const Base* b) {
    return polymorphicEq<Base, DerivedFirst>(a, b) ||
           polymorphicEq<Base, DerivedSecond, Derived...>(a, b);
}

}  // namespace utils
