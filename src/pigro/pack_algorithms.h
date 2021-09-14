#pragma once

#include <tuple>

namespace pigro {

template<size_t idx, typename T>
struct Item {
    explicit Item(T value) : value{ std::forward<T>(value) } {}

    T value;
    constexpr static auto index = idx;
};
template<typename... Pack, typename F>
auto enumerate_pack(F &&f, Pack &&...pack) {
    return [&, t = std::tuple<decltype(pack)...>{ std::forward<decltype(pack)>(pack)... } ]<size_t... idx>(std::index_sequence<idx...>) mutable {
        return f(
          Item<
            idx,
            std::tuple_element_t<idx, decltype(t)>>(
            std::forward<
              std::tuple_element_t<idx, decltype(t)>>(std::get<idx>(t)))...);
    }
    (std::make_index_sequence<sizeof...(pack)>{});
}

template<size_t n>
auto enumerate_n(auto &&f) {
    return [&]<size_t... idx>(std::index_sequence<idx...>) {
        return enumerate_pack(f, idx...);
    }
    (std::make_index_sequence<n>{});
}

} // namespace pigro
