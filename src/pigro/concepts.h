#pragma once

#include <type_traits>
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

template<typename T>
concept empty = std::is_empty_v<std::remove_cvref_t<T>>;

template<std::size_t I, class T>
concept has_tuple_element =
  requires {
    typename std::tuple_element<I, T>::type;
};

template<class T>
concept tuple_like = requires {
    typename std::tuple_size<std::remove_reference_t<T>>;
    std::same_as<decltype(std::tuple_size_v<std::remove_reference_t<T>>), size_t>;
}
&&[]<std::size_t... I>(std::index_sequence<I...>) {
    return (has_tuple_element<I, std::remove_reference_t<T>> && ...);
}
(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>());
;

} // namespace pigro::concepts
