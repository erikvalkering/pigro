#pragma once

#include <type_traits>

#define SFINAEABLE_RETURN(...) \
    ->decltype(__VA_ARGS__) {  \
        return __VA_ARGS__;    \
    }

namespace pigro {

template<size_t index>
using idx_t = std::integral_constant<size_t, index>;

template<size_t index>
constexpr auto idx = idx_t<index>{};

template<typename T>
constexpr decltype(auto) as_nonconst(T &value) noexcept {
    return const_cast<std::remove_cv_t<T> &>(value);
}

template<typename T>
constexpr decltype(auto) as_nonconst(T &&value) noexcept {
    return const_cast<std::remove_cv_t<T> &&>(value);
}

template<typename From, template<typename...> class To>
struct rebind_container;

template<template<typename...> class From, template<typename...> class To, typename... Ts>
struct rebind_container<From<Ts...>, To> {
    using type = To<Ts...>;
};

template<typename From, template<typename...> class To>
using rebind_container_t = rebind_container<From, To>::type;

} // namespace pigro
