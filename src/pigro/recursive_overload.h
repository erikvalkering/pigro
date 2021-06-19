#pragma once

#include "overload.h"
#include "recursive.h"

namespace pigro {

template<typename... Fs>
using recursive_overload = recursive<overload<Fs...>>;

template<typename... Fs>
recursive_overload(Fs...) -> recursive_overload<Fs...>;

} // namespace pigro
