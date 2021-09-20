#pragma once

#include <tuple>

namespace pigro {

// Credits go to the article:
// "capturing perfectly-forwarded objects in lambdas",
// by Vittorio Romeo, 11 december 2016
// https://vittorioromeo.info/index/blog/capturing_perfectly_forwarded_objects_in_lambdas.html

template<typename... Ts>
auto fwd_capture(Ts &&...xs) {
    return std::tuple<Ts...>{ std::forward<Ts>(xs)... };
}

template<typename T>
decltype(auto) access(T &&x) {
    return std::get<0>(std::forward<T>(x));
}

template<typename... Ts>
auto fwd_capture_as_tuple(Ts &&...xs) {
    return std::make_tuple(fwd_capture(std::forward<Ts>(xs)...));
}

} // namespace pigro
