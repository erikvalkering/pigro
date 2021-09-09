#pragma once

#include "concepts.h"
#include "empty_object.h"
#include "overload.h"
#include "pack_algorithms.h"
#include "recursive.h"
#include "utils.h"

#include <tuple>
#include <type_traits>
#include <utility>

namespace pigro {

template<size_t tag, concepts::empty T>
auto compressed_tuple_element(T value) {
    return overload{
        [](const auto &self, idx_t<tag>) {
            return empty_object<T>::get();
        },
        [](auto &self, idx_t<tag>) mutable {
            return empty_object<T>::get();
        },
    };
}

template<size_t tag, typename T>
auto compressed_tuple_element(T &&value) {
    return overload{
        [](const auto &self, idx_t<tag>) -> const T & {
            return const_cast<std::remove_cvref_t<decltype(self)> &>(self)(idx<tag>);
        },
        // TODO: capture is int, but should be int &
        // TODO: if moved into the tuple, then should be int
        [capture = std::tuple{ std::forward<T>(value) }](auto &self, idx_t<tag>) mutable -> T & {
            return std::get<0>(capture);
        },
    };
}

auto make_compressed_tuple_base(auto &&...values) {
    return enumerate_pack(
      [](auto &&...items) {
          return recursive{
              overload{ compressed_tuple_element<items.index>(std::forward<decltype(items)>(items).value)... }
          };
      },
      std::forward<decltype(values)>(values)...);
}

template<typename... Ts>
using compressed_tuple_base_t = decltype(make_compressed_tuple_base(std::declval<Ts>()...));

template<typename... Ts>
struct compressed_tuple : compressed_tuple_base_t<Ts...> {
    explicit compressed_tuple(Ts &&...values)
      : compressed_tuple_base_t<Ts...>{ make_compressed_tuple_base(std::forward<decltype(values)>(values)...) } {
    }
};

compressed_tuple()->compressed_tuple<>;

template<typename... Ts>
compressed_tuple(Ts &&...) -> compressed_tuple<Ts...>;

template<typename... Ts>
compressed_tuple(Ts &...) -> compressed_tuple<Ts &...>;

} // namespace pigro

namespace std {

template<typename... Ts>
struct tuple_size<pigro::compressed_tuple<Ts...>>
  : std::integral_constant<size_t, sizeof...(Ts)> {
};

template<typename... Ts>
struct tuple_size<const pigro::compressed_tuple<Ts...>>
  : std::integral_constant<size_t, sizeof...(Ts)> {
};

template<typename... Ts>
struct tuple_size<pigro::compressed_tuple<Ts...> &&>
  : std::integral_constant<size_t, sizeof...(Ts)> {
};

template<typename... Ts>
struct tuple_size<const pigro::compressed_tuple<Ts...> &&>
  : std::integral_constant<size_t, sizeof...(Ts)> {
};

template<size_t I, typename... Ts>
constexpr decltype(auto) get(pigro::compressed_tuple<Ts...> &t) {
    return t(pigro::idx<I>);
}

template<size_t I, typename... Ts>
constexpr decltype(auto) get(const pigro::compressed_tuple<Ts...> &t) {
    return t(pigro::idx<I>);
}

template<size_t I, typename... Ts>
struct tuple_element<I, pigro::compressed_tuple<Ts...>> {
    using type = decltype(std::get<I>(std::declval<pigro::compressed_tuple<Ts...>>()));
};

} // namespace std
