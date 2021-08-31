#pragma once

#define SFINAEABLE_RETURN(...) \
    ->decltype(__VA_ARGS__) {  \
        return __VA_ARGS__;    \
    }

namespace pigro {

template<size_t index>
using idx_t = std::integral_constant<size_t, index>;

template<size_t index>
constexpr auto idx = idx_t<index>{};

} // namespace pigro
