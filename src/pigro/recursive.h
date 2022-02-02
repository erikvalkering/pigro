#pragma once

#include "forward_like.h"
#include "utils.h"

#include <concepts>
#include <utility>

namespace pigro {

template<typename F>
struct recursive : F {
    static_assert(!std::is_const_v<F>);
    static_assert(!std::is_volatile_v<F>);
    static_assert(!std::is_reference_v<F>);

    constexpr decltype(auto) operator()(auto &&...args) & requires std::invocable<F &, recursive &, decltype(args)...> {
        return std::invoke(static_cast<F &>(*this), *this, FWD(args)...);
    }

    constexpr decltype(auto) operator()(auto &&...args) && requires std::invocable<F &&, recursive &&, decltype(args)...> {
        return std::invoke(static_cast<F &&>(*this), std::move(*this), FWD(args)...);
    }

    constexpr decltype(auto) operator()(auto &&...args) const &requires std::invocable<const F &, const recursive &, decltype(args)...> {
        return std::invoke(static_cast<const F &>(*this), *this, FWD(args)...);
    }

    constexpr decltype(auto) operator()(auto &&...args) const &&requires std::invocable<const F &&, const recursive &&, decltype(args)...> {
        return std::invoke(static_cast<const F &&>(*this), std::move(*this), FWD(args)...);
    }
};

} // namespace pigro
