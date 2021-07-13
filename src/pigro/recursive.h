#pragma once

#include "utils.h"

#include <utility>

namespace pigro {

template<typename F>
struct recursive : F {
    template<typename... Args>
    constexpr auto operator()(Args &&...args) const
      SFINAEABLE_RETURN(F::operator()(*this, std::forward<Args>(args)...));

    template<typename... Args>
    constexpr auto operator()(Args &&...args)
      SFINAEABLE_RETURN(F::operator()(*this, std::forward<Args>(args)...));
};

template<typename F>
recursive(F) -> recursive<F>;

} // namespace pigro
