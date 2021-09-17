#pragma once

#include <tuple>
#include <type_traits>


namespace pigro {

template<size_t... idx>
auto apply_impl(auto &&f, auto &&t, std::index_sequence<idx...>) -> decltype(std::invoke(std::forward<decltype(f)>(f), get<idx>(std::forward<decltype(t)>(t))...)) {
    return std::invoke(std::forward<decltype(f)>(f), get<idx>(std::forward<decltype(t)>(t))...);
}

auto apply(auto &&f, auto &&t)
  -> decltype(apply_impl(std::forward<decltype(f)>(f), std::forward<decltype(t)>(t), std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<decltype(t)>>>{})) {
    return apply_impl(std::forward<decltype(f)>(f), std::forward<decltype(t)>(t), std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<decltype(t)>>>{});
}

} // namespace pigro
