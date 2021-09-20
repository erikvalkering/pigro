#pragma once

#include "apply.h"
#include "concepts.h"

#include <tuple>
#include <utility>

namespace pigro {

auto to_tuple(concepts::tuple_like auto &&t) {
    return pigro::apply(
      [](auto &&...values) {
          return std::forward_as_tuple(std::forward<decltype(values)>(values)...);
      },
      std::forward<decltype(t)>(t));
}

} // namespace pigro
