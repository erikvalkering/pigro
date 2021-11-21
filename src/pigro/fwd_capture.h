#pragma once

#include <tuple>

namespace pigro {

// Credits go to the article:
// "capturing perfectly-forwarded objects in lambdas",
// by Vittorio Romeo, 11 december 2016
// https://vittorioromeo.info/index/blog/capturing_perfectly_forwarded_objects_in_lambdas.html

template<typename T>
auto fwd_capture(T &&x) {
    return std::tuple<T>{ std::forward<T>(x) };
}

template<typename T>
decltype(auto) access(T &&x) {
    return std::get<0>(std::forward<T>(x));
}

} // namespace pigro

#define FWD_CAPTURE(x) fwd_capture(std::forward<decltype(x)>(x))
