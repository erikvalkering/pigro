#pragma once

#include "concepts.h"
#include "overload.h"
#include "recursive.h"
#include "utils.h"

#include <tuple>

namespace pigro {

template<size_t tag>
auto compressed_tuple_element(concepts::empty auto value) {
    return [](auto &&self, idx_t<tag>) {
        return decltype(value){};
    };
}

template<size_t tag>
auto compressed_tuple_element(auto value) {
    return
      [=](auto &&self, idx_t<tag>) mutable -> auto & {
        return value;
    };
}

auto make_compressed_tuple_base(auto... values) {
    return enumerate_pack(
      [](auto... items) {
          return recursive{
              overload{
                compressed_tuple_element<decltype(items)::index>(
                  items.value)... }
          };
      },
      values...);
}

template<typename... Ts>
using compressed_tuple_base_t = decltype(make_compressed_tuple_base(std::declval<Ts>()...));

template<typename... Ts>
struct compressed_tuple : compressed_tuple_base_t<Ts...> {
    explicit compressed_tuple(Ts... values)
      : compressed_tuple_base_t<Ts...>{ make_compressed_tuple_base(values...) } {
    }
};

} // namespace pigro

namespace std {

template<typename... Ts>
struct tuple_size<pigro::compressed_tuple<Ts...>>
  : std::integral_constant<size_t, sizeof...(Ts)> {
};

template<size_t I, typename... Ts>
constexpr auto &&get(pigro::compressed_tuple<Ts...> t) {
    return t(pigro::idx<I>);
}

template<size_t I, typename... Ts>
struct tuple_element<I, pigro::compressed_tuple<Ts...>> {
    using type = decltype(std::get<I>(std::declval<pigro::compressed_tuple<Ts...>>()));
};

} // namespace std
