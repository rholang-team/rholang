#pragma once

#include <concepts>
#include <utility>

namespace utils {
template <typename T, typename U>
bool isa(const U* p) {
    return nullptr != dynamic_cast<const T*>(p);
}

template <typename R, typename Derived, typename Base, typename F>
    requires std::invocable<F, Derived&> &&
             std::convertible_to<std::invoke_result_t<F, Derived&>, R>
R match(Base* x, F f) {
    if (Derived* p = dynamic_cast<Derived*>(x)) {
        return f(*p);
    }

    std::unreachable();
}
}  // namespace utils
