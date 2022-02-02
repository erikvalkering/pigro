#pragma once

#include "apply.h"
#include "compressed_tuple.h"
#include "concepts.h"
#include "fwd_capture.h"
#include "pack_algorithms.h"
#include "recursive.h"

#include <concepts>
#include <functional>

namespace pigro {

// clang-format off
template<typename F>
auto bind_back(F &&f, auto &&...back_args) {
    return recursive{
        overload{
            [f = FWD_CAPTURE(f), ... back_args = FWD_CAPTURE(back_args)](auto &&self, auto &&...front_args) -> decltype(auto)
                requires std::invocable<pigro::like_t<decltype(self), F>, decltype(front_args)..., decltype(back_args)...> {
                return access(f)(FWD(front_args)..., access(back_args)...);
            },
            [f = FWD_CAPTURE(f), ... back_args = FWD_CAPTURE(back_args)](auto &&self, auto &&...front_args) mutable -> decltype(auto)
                requires std::invocable<pigro::like_t<decltype(self), F>, decltype(front_args)..., decltype(back_args)...> {
                return access(f)(FWD(front_args)..., access(back_args)...);
            },
        }
    };
}
// clang-format on

auto bind_front_tuple(auto &&f, concepts::tuple_like auto &&t) {
    return recursive{
        overload{
          compressed_tuple_element<0>(FWD(f)),
          compressed_tuple_element<1>(FWD(t)),
          [](auto &&self, auto &&...args)
            -> decltype(pigro::apply(bind_back(f, FWD(args)...), t)) {
              auto &&f = self(idx<0>);
              auto &&t = self(idx<1>);

              return pigro::apply(bind_back(f, FWD(args)...), t);
          },
        }
    };
}

auto bind_back_tuple(auto &&f, concepts::tuple_like auto &&t) {
    return recursive{
        overload{
          compressed_tuple_element<0>(FWD(f)),
          compressed_tuple_element<1>(FWD(t)),
          [](auto &&self, auto &&...args)
            -> decltype(pigro::apply(std::bind_front(f, FWD(args)...), t)) {
              auto &&f = self(idx<0>);
              auto &&t = self(idx<1>);

              return pigro::apply(std::bind_front(f, FWD(args)...), t);
          },
        }
    };
}

auto operator>>(concepts::tuple_like auto &&args, auto &&f) {
    return bind_front_tuple(FWD(f), FWD(args));
}

auto operator<<(concepts::tuple_like auto &&args, auto &&f) {
    return bind_back_tuple(FWD(f), FWD(args));
}

} // namespace pigro
