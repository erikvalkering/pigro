#pragma once

#include "overload.h"
#include "recursive.h"
#include "regular_void.h"
#include "tuple_algorithms.h"

#include <concepts>
#include <cstddef>
#include <optional>
#include <utility>

namespace pigro::concepts {

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

template<typename F>
concept lazy_function_unwrapped = lazy_function<F> && ::std::invocable<F>;

} // namespace pigro::concepts

namespace pigro::detail {

template<typename T>
struct LazyResult {
    T value;
    bool is_changed;
};

template<typename T>
LazyResult(const T &, bool) -> LazyResult<const T &>;
LazyResult(int &&, bool)->LazyResult<int>;

constexpr auto value = [](const concepts::lazy_result auto result) {
    return result.value;
};

constexpr auto is_changed = [](const concepts::lazy_result auto result) {
    return result.is_changed;
};

constexpr auto unwrap_value(concepts::lazy_function auto f) {
    return recursive{
        overload{
          [=](auto &, std::nullptr_t) mutable {
              return f(nullptr);
          },
          [](auto &self) {
              return unregularize_void(self(nullptr).value);
          },
        }
    };
}

constexpr auto lazy(auto f, concepts::lazy_function auto... deps) {
    auto ff = regularize_void(f);

    using result_t = decltype(ff(deps(nullptr).value...));

    auto cache = std::optional<result_t>{};
    return unwrap_value([=](std::nullptr_t) mutable {
        const auto args = std::tuple{ deps(nullptr)... };

        auto changed = !cache || any(args, is_changed);
        if (changed) {
            const auto values = transform(args, value);
            const auto result = apply(ff, values);

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

constexpr auto ensure_lazy(concepts::lazy_function_unwrapped auto dep) {
    return dep;
}

constexpr auto ensure_lazy(::std::invocable auto dep) {
    return detail::lazy(
      [=](auto) mutable { return dep(); },
      lazy_value(0, std::true_type{}));
}

constexpr auto ensure_lazy(auto dep) {
    return lazy_value(dep, std::false_type{});
}

} // namespace pigro::detail

namespace pigro {

auto lazy(auto f, auto... deps) {
    return detail::lazy(f, detail::ensure_lazy(deps)...);
}

} // namespace pigro
