#pragma once

#include "utils.h"

#include <utility>

namespace pigro {

template<typename F>
struct recursive_impl;

template<typename F>
concept has_operator_call = requires {
    {
        F::operator()
    };
};

template<has_operator_call F>
struct recursive_impl<F> : F {
    using F::operator();

    template<typename... Args>
    constexpr decltype(auto) operator_call(Args &&...args) {
        return (*this)(std::forward<Args>(args)...);
    }

    template<typename... Args>
    constexpr decltype(auto) operator_call(Args &&...args) const {
        return (*this)(std::forward<Args>(args)...);
    }
};

template<typename F>
struct recursive_impl {
    explicit recursive_impl(...) {}
    constexpr auto operator_call(...) const;
};

template<typename F>
struct recursive : private recursive_impl<F> {
    static_assert(!std::is_const_v<F>);
    static_assert(!std::is_reference_v<F>);
    explicit recursive(F f) : recursive_impl<F>{ f } {}

    using recursive_impl<F>::operator_call;

    template<typename... Args>
    constexpr decltype(auto) operator()(Args &&...args) {
        return operator_call(*this, std::forward<Args>(args)...);
    }

    template<typename... Args>
    constexpr decltype(auto) operator()(Args &&...args) const {
        return operator_call(*this, std::forward<Args>(args)...);
    }
};

template<typename F>
recursive(F) -> recursive<F>;

} // namespace pigro
