#pragma once

#include "overload.h"
#include "recursive.h"

#include <concepts>
#include <cstddef>
#include <optional>
#include <utility>

namespace pigro::detail {

template<typename T>
struct LazyResult {
    T value;
    bool is_changed;
};

template<typename T>
LazyResult(const T &, bool) -> LazyResult<const T &>;
LazyResult(int &&, bool)->LazyResult<int>;

template<typename T>
concept lazy_result = requires(T t) {
    t.value;
    t.is_changed;
};

template<typename F>
concept lazy_function = requires(F f) {
    { f(nullptr) }
        -> lazy_result;
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

constexpr auto ensure_lazy(lazy_function auto dep) {
    return dep;
}

constexpr auto ensure_lazy(std::invocable auto dep) {
    return detail::lazy(
      [=](auto) mutable { return dep(); },
      lazy_value(0, std::true_type{}));
}

constexpr auto ensure_lazy(auto dep) {
    return lazy_value(dep, std::false_type{});
}

} // namespace pigro::detail

namespace pigro {

auto lazy(auto f, auto dep) {
    return detail::lazy(f, detail::ensure_lazy(dep));
}

} // namespace pigro
