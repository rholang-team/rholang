#pragma once

#include <forward_list>
#include <optional>
#include <unordered_map>

namespace frontend {
template <typename K, typename V>
class ScopedMap {
    std::forward_list<std::unordered_map<K, V>> scopes_;

public:
    void pushScope() {
        scopes_.emplace_front();
    }

    void popScope() {
        scopes_.pop_front();
    }

    template <typename T>
        requires std::convertible_to<T, V>
    void addOrShadow(const K& k, T&& v) {
        scopes_.front().insert_or_assign(k, std::forward<T>(v));
    }

    template <typename T>
        requires std::convertible_to<T, V>
    void addOrShadow(K&& k, T&& v) {
        scopes_.front().insert_or_assign(std::move(k), std::forward<T>(v));
    }

    std::optional<std::reference_wrapper<V const>> lookup(const K& k) const {
        for (const auto& scope : scopes_) {
            auto it = scope.find(k);
            if (it != scope.end())
                return it->second;
        }

        return std::nullopt;
    }
};
}  // namespace frontend
