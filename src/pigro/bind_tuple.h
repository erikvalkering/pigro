#pragma once

#include "compressed_tuple.h"
#include "concepts.h"
#include "pack_algorithms.h"

namespace pigro {

auto bind_front_tuple(auto &&f, concepts::tuple_like auto &&t) {
    return recursive{
        overload{
          compressed_tuple_element<0>(std::forward<decltype(f)>(f)),
          compressed_tuple_element<1>(std::forward<decltype(t)>(t)),
          [](auto &&self, auto &&...args) {
              auto &&f = self(idx<0>);
              auto &&compressed_args = self(idx<1>);

              return enumerate_n<std::tuple_size_v<std::remove_cvref_t<decltype(compressed_args)>>>(
                [&](auto... items) {
                    return f(std::get<items.index>(compressed_args)..., std::forward<decltype(args)>(args)...);
                });
          },
        }
    };
}

auto operator>>(concepts::tuple_like auto &&args, auto &&f) {
    return bind_front_tuple(std::forward<decltype(f)>(f), std::forward<decltype(args)>(args));
}

} // namespace pigro
