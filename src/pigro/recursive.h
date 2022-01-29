#pragma once

#include "forward_like.h"
#include "utils.h"

#include <utility>

namespace pigro {

template<typename F>
struct recursive : F {
    static_assert(!std::is_const_v<F>);
    static_assert(!std::is_volatile_v<F>);
    static_assert(!std::is_reference_v<F>);

    template<typename... Args>
    constexpr auto operator()(Args &&...args) & -> decltype(std::invoke(static_cast<F &>(*this), *this, std::forward<Args>(args)...)) {
        return std::invoke(static_cast<F &>(*this), *this, std::forward<Args>(args)...);
    }

    template<typename... Args>
    constexpr auto operator()(Args &&...args) && -> decltype(std::invoke(static_cast<F &&>(*this), std::move(*this), std::forward<Args>(args)...)) {
        return std::invoke(static_cast<F &&>(*this), std::move(*this), std::forward<Args>(args)...);
    }

    template<typename... Args>
    constexpr auto operator()(Args &&...args) const & -> decltype(std::invoke(static_cast<const F &>(*this), *this, std::forward<Args>(args)...)) {
        return std::invoke(static_cast<const F &>(*this), *this, std::forward<Args>(args)...);
    }

    template<typename... Args>
    constexpr auto operator()(Args &&...args) const && -> decltype(std::invoke(static_cast<const F &&>(*this), std::move(*this), std::forward<Args>(args)...)) {
        return std::invoke(static_cast<const F &&>(*this), std::move(*this), std::forward<Args>(args)...);
    }
};

} // namespace pigro
