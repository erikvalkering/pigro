#pragma once

#include <utility>

namespace pigro {

template<typename F>
struct recursive : F {
    template<typename... Args>
    constexpr decltype(auto) operator()(Args &&...args) const {
        return F::operator()(*this, std::forward<Args>(args)...);
    }

    template<typename... Args>
    constexpr decltype(auto) operator()(Args &&...args) {
        return F::operator()(*this, std::forward<Args>(args)...);
    }
};

template<typename F>
recursive(F) -> recursive<F>;

} // namespace pigro
