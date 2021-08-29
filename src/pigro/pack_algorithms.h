#pragma once

#include <tuple>

namespace pigro {

template<size_t idx, typename T>
struct Enumerator {
    T value;
    constexpr static auto index = idx;
};

auto enumerate_pack(auto f, auto... pack) {
    return [ =, t = std::make_tuple(pack...) ]<size_t... idx>(std::index_sequence<idx...>) {
        return f(Enumerator<idx, decltype(get<idx>(t))>{ std::get<idx>(t) }...);
    }
    (std::make_index_sequence<sizeof...(pack)>{});
}

auto enumerate_tuple(auto f, auto t) {
    return std::apply([=](auto... pack) { return enumerate_pack(f, pack...); }, t);
}

template<size_t n>
auto enumerate_n(auto f) {
    return [=]<size_t... idx>(std::index_sequence<idx...>) {
        return enumerate_pack(f, idx...);
    }
    (std::make_index_sequence<n>{});
}

} // namespace pigro
