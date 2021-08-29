#pragma once

#include <type_traits>

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

template<typename T>
concept empty = std::is_empty_v<T>;

} // namespace pigro::concepts
