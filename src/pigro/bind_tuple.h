#pragma once

#include "apply.h"
#include "compressed_tuple.h"
#include "concepts.h"
#include "pack_algorithms.h"

namespace pigro {

auto bind_back(auto &&f, auto... back_args) {
    return [=, f = std::forward<decltype(f)>(f)](auto &&...front_args) {
        return std::invoke(f, std::forward<decltype(front_args)>(front_args)..., back_args...);
    };
}

auto bind_front_tuple(auto &&f, concepts::tuple_like auto &&t) {
    return recursive{
        overload{
          compressed_tuple_element<0>(std::forward<decltype(f)>(f)),
          compressed_tuple_element<1>(std::forward<decltype(t)>(t)),
          [](auto &&self, auto &&...args) -> decltype(pigro::apply(f, t)) {
              auto &&f = self(idx<0>);
              auto &&t = self(idx<1>);

              return pigro::apply(f, t);
          },
        }
    };
}

auto operator>>(concepts::tuple_like auto &&args, auto &&f) {
    return bind_front_tuple(std::forward<decltype(f)>(f), std::forward<decltype(args)>(args));
}

} // namespace pigro
