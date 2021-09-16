#pragma once

#include <type_traits>

namespace pigro {

// Credits go to: https://github.com/atomgalaxy/isocpp-template-this/blob/master/forward_like.cpp

// given a facility that transfers cv-qualifiers from type to type
// clang-format off
template <typename From, typename To>
class like {
  template <bool Condition, template <typename> class Function, typename T>
  using apply_if = std::conditional_t<Condition, Function<T>, T>;
  using base = std::remove_cv_t<std::remove_reference_t<To>>;
  using base_from = std::remove_reference_t<From>;

  static constexpr bool rv = std::is_rvalue_reference_v<From>;
  static constexpr bool lv = std::is_lvalue_reference_v<From>;
  static constexpr bool c = std::is_const_v<base_from>;
  static constexpr bool v = std::is_volatile_v<base_from>;

public:
  using type = apply_if<lv, std::add_lvalue_reference_t,
               apply_if<rv, std::add_rvalue_reference_t,
               apply_if<c, std::add_const_t,
               apply_if<v, std::add_volatile_t,
               base>>>>;
};

template <typename From, typename To>
using like_t = typename like<From, To>::type;
// clang-format on

// ... we can define forward_like like so:
template<typename Like, typename T>
constexpr decltype(auto) forward_like(T &&t) noexcept {
    // first, get `t` back into the value category it was passed in
    // then, forward it as if its value category was `Like`'s.
    // This prohibits rvalue -> lvalue conversions.
    return std::forward<like_t<Like, T>>(std::forward<T>(t));
}

} // namespace pigro
