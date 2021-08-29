#pragma once

#include "compressed_tuple.h"
#include "concepts.h"

namespace pigro {

auto bind_front_tuple(auto f, concepts::tuple_like auto t) {
    return recursive{
        overload{
          compressed_tuple_element<0>(f),
          compressed_tuple_element<1>(t),
          [](auto &self) mutable {
              auto &&f = self(idx<0>);
              auto &&compressed_args = self(idx<1>);

              return std::apply(f, compressed_args);
          },
        }
    };
}

auto operator>>(concepts::tuple_like auto args, auto f) {
    return bind_front_tuple(f, args);
}

} // namespace pigro
