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
requires std::is_rvalue_reference_v<T &&>
auto compressed_tuple_element(T &&value) {
    return [](const auto &self, idx_t<tag>) {
        return empty_object<std::remove_cvref_t<T>>::get();
    };
}

template<typename T>
using capture_t = std::conditional_t<std::is_lvalue_reference_v<T>, T, std::remove_reference_t<T>>;

template<size_t tag, typename T>
auto compressed_tuple_element(T &&value) {
    return overload{
        [](const auto &self, idx_t<tag>) -> decltype(std::as_const(as_nonconst(self)(as_nonconst(self), idx<tag>))) {
            return std::as_const(as_nonconst(self)(as_nonconst(self), idx<tag>));
        },
        [capture = std::tuple<capture_t<T>>{ std::forward<T>(value) }](auto &self, idx_t<tag>) mutable -> decltype(std::get<0>(std::declval<decltype(std::tuple<capture_t<T>>{ std::forward<T>(value) }) &>())) {
            return std::get<0>(capture);
        },
    };
}

auto make_compressed_tuple_base(auto &&...values) {
    return enumerate_pack(
      [](auto &&...items) {
          return recursive{
              overload{ compressed_tuple_element<items.index>(std::forward<decltype(items.value)>(items.value))... }
          };
      },
      std::forward<decltype(values)>(values)...);
}

template<typename... Ts>
using compressed_tuple_base_t = decltype(make_compressed_tuple_base(std::declval<Ts>()...));

template<typename... Ts>
struct compressed_tuple : compressed_tuple_base_t<Ts...> {
    compressed_tuple() requires(std::default_initializable<Ts> &&...)
      : compressed_tuple_base_t<Ts...>{ make_compressed_tuple_base(Ts{}...) } {}

    explicit compressed_tuple(Ts &&...values)
      : compressed_tuple_base_t<Ts...>{ make_compressed_tuple_base(std::forward<decltype(values)>(values)...) } {
    }
};

template<typename... Ts>
compressed_tuple(Ts &&...) -> compressed_tuple<capture_t<Ts>...>;

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
