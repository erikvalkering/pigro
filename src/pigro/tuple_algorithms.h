#pragma once

#include <functional>
#include <tuple>

namespace pigro {

constexpr auto transform(const auto &data, auto projection) {
    return std::apply([=](const auto &...args) {
        return std::tuple{ projection(args)... };
    },
      data);
}

template<typename Predicate = std::identity>
constexpr auto any(const auto &data, Predicate predicate = {}) {
    return std::apply([=](const auto &...args) {
        return (predicate(args) || ...);
    },
      data);
}

} // namespace pigro
