#pragma once

#include <concepts>

template <typename T>
class IListNode {
    T* prev_;
    T* next_;

protected:
    IListNode(T* prev, T* next) : prev_{prev}, next_{next} {}

public:
    T* prev() const;
    T* next() const;
};

template <typename T>
    requires std::derived_from<T, IListNode<T>>
class IList {
    T* head_;
    T* tail_;

public:
    // TODO
};
