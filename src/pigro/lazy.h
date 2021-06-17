#pragma once

#include <optional>

namespace pigro {

constexpr auto lazy = [](auto f, auto dep) {
    using result_t = decltype(f(dep()));
    using dep_result_t = decltype(dep());

    auto cache = std::optional<result_t>{};
    auto dep_cache = std::optional<dep_result_t>{};
    return [=]() mutable {
        const auto arg = dep();
        if (!cache || arg != dep_cache) {
            dep_cache = arg;
            cache = f(*dep_cache);
        }

        return *cache;
    };
};

} // namespace pigro
