#pragma once

#include "apply.h"
#include "compressed_tuple.h"
#include "concepts.h"
#include "fwd_capture.h"
#include "pack_algorithms.h"

#include <functional>

namespace pigro {

auto bind_back(auto &&f, auto &&...back_args) {
    return [f = FWD_CAPTURE(f), ... back_args = FWD_CAPTURE(back_args)](auto &&...front_args) mutable
           -> decltype(std::invoke(FWD(f), FWD(front_args)..., FWD(back_args)...)) {
        return std::invoke(access(f), FWD(front_args)..., access(back_args)...);
    };
}

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
