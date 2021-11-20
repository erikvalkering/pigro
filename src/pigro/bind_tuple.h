#pragma once

#include "apply.h"
#include "compressed_tuple.h"
#include "concepts.h"
#include "fwd_capture.h"
#include "pack_algorithms.h"

#include <functional>

namespace pigro {

auto bind_back(auto &&f, auto &&...back_args) {
    return [f = fwd_capture(std::forward<decltype(f)>(f)), ... back_args = fwd_capture(std::forward<decltype(back_args)>(back_args))](auto &&...front_args) mutable
           -> decltype(std::invoke(std::forward<decltype(f)>(f), std::forward<decltype(front_args)>(front_args)..., std::forward<decltype(back_args)>(back_args)...)) {
        return std::invoke(access(f), std::forward<decltype(front_args)>(front_args)..., access(back_args)...);
    };
}

auto bind_front_tuple(auto &&f, concepts::tuple_like auto &&t) {
    return recursive{
        overload{
          compressed_tuple_element<0>(std::forward<decltype(f)>(f)),
          compressed_tuple_element<1>(std::forward<decltype(t)>(t)),
          [](auto &&self, auto &&...args)
            -> decltype(pigro::apply(bind_back(f, std::forward<decltype(args)>(args)...), t)) {
              auto &&f = self(idx<0>);
              auto &&t = self(idx<1>);

              return pigro::apply(bind_back(f, std::forward<decltype(args)>(args)...), t);
          },
        }
    };
}

auto bind_back_tuple(auto &&f, concepts::tuple_like auto &&t) {
    return recursive{
        overload{
          compressed_tuple_element<0>(std::forward<decltype(f)>(f)),
          compressed_tuple_element<1>(std::forward<decltype(t)>(t)),
          [](auto &&self, auto &&...args) {
              auto &&f = self(idx<0>);
              auto &&t = self(idx<1>);

              return pigro::apply(std::bind_front(f, std::forward<decltype(args)>(args)...), t);
          },
        }
    };
}

auto operator>>(concepts::tuple_like auto &&args, auto &&f) {
    return bind_front_tuple(std::forward<decltype(f)>(f), std::forward<decltype(args)>(args));
}

auto operator<<(concepts::tuple_like auto &&args, auto &&f) {
    return bind_back_tuple(std::forward<decltype(f)>(f), std::forward<decltype(args)>(args));
}

} // namespace pigro
