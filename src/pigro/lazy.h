#pragma once

#include "overload.h"
#include "recursive.h"

#include <concepts>
#include <cstddef>
#include <optional>
#include <utility>

namespace pigro {

template<typename T>
struct LazyResult {
    T value;
    bool is_changed;
};

template<typename T>
LazyResult(const T &, bool) -> LazyResult<const T &>;
LazyResult(int &&, bool)->LazyResult<int>;

template<typename F>
concept lazy_function = requires(F f) {
    f(nullptr);
};

constexpr auto unwrap(lazy_function auto lazy_f) {
    return recursive{
        overload{
          [=](auto &, std::nullptr_t) mutable {
              return lazy_f(nullptr);
          },
          [](auto &self) {
              return self(nullptr).value;
          },
        }
    };
}

constexpr auto lazy(auto f, lazy_function auto dep) {
    using result_t = decltype(f(dep(nullptr).value));

    auto cache = std::optional<result_t>{};
    return unwrap([=](std::nullptr_t) mutable {
        const auto arg = dep(nullptr);

        auto changed = !cache || arg.is_changed;
        if (changed) {
            const auto value = arg.value;
            const auto result = f(value);

            changed = cache != result;
            cache = std::move(result);
        }

        return LazyResult{
            *cache,
            changed,
        };
    });
}

constexpr auto lazy_value(auto value, auto changed) {
    return [=](std::nullptr_t) {
        return LazyResult{
            value,
            changed,
        };
    };
};

constexpr auto lazy_always_reevaluate(std::invocable auto f) {
    return lazy(
      [=](auto) mutable { return f(); },
      lazy_value(0, std::true_type{}));
};

auto lazy(auto f, auto dep) {
    return lazy(f, lazy_always_reevaluate(dep));
}

} // namespace pigro
