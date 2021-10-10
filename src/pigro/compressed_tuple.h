#pragma once

#include "concepts.h"
#include "empty_object.h"
#include "forward_like.h"
#include "fwd_capture.h"
#include "overload.h"
#include "pack_algorithms.h"
#include "recursive.h"
#include "utils.h"

#include <tuple>
#include <type_traits>
#include <utility>

namespace pigro {

template<size_t tag, concepts::empty T>
auto compressed_tuple_element(T) {
    return [](auto &&, idx_t<tag>) {
        return empty_object<std::remove_cvref_t<T>>::get();
    };
}

template<size_t tag, typename T>
auto compressed_tuple_element(T &&value) {
    return overload{
        [](auto &&self, idx_t<tag>) -> decltype(auto) {
            return forward_like<decltype(self)>(as_nonconst(std::forward<decltype(self)>(self))(idx<tag>));
        },
        [value = fwd_capture(std::forward<T>(value))](auto &&self, idx_t<tag>) mutable -> decltype(auto) {
            return access(forward_like<decltype(self)>(value));
        },
    };
}

template<typename... Ts>
auto make_compressed_tuple_base(Ts &&...values) {
    return enumerate_pack(
      [](const auto &...items) {
          return recursive{
              overload{ compressed_tuple_element<items.index>(std::forward<decltype(items.value)>(items.value))... }
          };
      },
      std::forward<decltype(values)>(values)...);
}

template<typename... Ts>
using compressed_tuple_base_t = decltype(make_compressed_tuple_base<Ts...>(std::declval<Ts>()...));

template<typename... Ts>
struct compressed_tuple : compressed_tuple_base_t<Ts...> {
    compressed_tuple() requires(std::default_initializable<Ts> &&...)
      : compressed_tuple_base_t<Ts...>{ make_compressed_tuple_base<Ts...>(Ts{}...) } {}

    template<typename... Us>
    explicit compressed_tuple(Us &&...values) requires(sizeof...(Ts) == sizeof...(Us))
      : compressed_tuple_base_t<Ts...>{
            make_compressed_tuple_base<Ts...>(static_cast<Ts...>(values)...)
        } {}
};

template<typename... Ts>
compressed_tuple(Ts &&...) -> compressed_tuple<std::remove_cvref_t<Ts>...>;

} // namespace pigro

namespace pigro::concepts {

template<typename>
constexpr auto is_compressed_tuple = false;

template<typename... Ts>
constexpr auto is_compressed_tuple<pigro::compressed_tuple<Ts...>> = true;

template<typename T>
concept compressed_tuple = is_compressed_tuple<std::remove_cvref_t<T>>;

} // namespace pigro::concepts

namespace pigro {

template<size_t I, concepts::compressed_tuple T>
constexpr auto get(T &&t) -> decltype(std::forward<T>(t)(pigro::idx<I>)) {
    return std::forward<T>(t)(pigro::idx<I>);
}

} // namespace pigro

namespace std {

template<pigro::concepts::compressed_tuple T>
struct tuple_size<T> : std::tuple_size<pigro::rebind_container_t<std::remove_cvref_t<T>, std::tuple>> {
};

template<size_t I, pigro::concepts::compressed_tuple T>
struct tuple_element<I, T> : std::tuple_element<I, pigro::rebind_container_t<std::remove_cvref_t<T>, std::tuple>> {
};

} // namespace std
