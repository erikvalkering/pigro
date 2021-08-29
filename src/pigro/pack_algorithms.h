#pragma once

#include <tuple>

namespace pigro {

template<size_t idx, typename T>
struct Enumerator {
    T value;
    constexpr static auto index = idx;
};

auto enumerate_pack(auto f, auto... pack) {
    return [ =, t = std::tuple{ pack... } ]<size_t... idx>(std::index_sequence<idx...>) {
        return f(Enumerator<idx, decltype(get<idx>(t))>{ get<idx>(t) }...);
    }
    (std::make_index_sequence<sizeof...(pack)>{});
}

} // namespace pigro
