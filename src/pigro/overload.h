#pragma once

#ifdef _MSC_VER
#include "partition.h"
#include <tuple>
#include <type_traits>
#endif

namespace pigro {

#ifdef _MSC_VER
template<typename>
struct __declspec(empty_bases) overload_impl;

template<typename... Fs>
struct __declspec(empty_bases) overload_impl<typelist<Fs...>> : Fs... {
    using Fs::operator()...;
};

template<typename... Fs>
struct __declspec(empty_bases) overload : overload_impl<partition_t<std::is_empty, Fs...>> {
    using base = overload_impl<partition_t<std::is_empty, Fs...>>;

    using base::operator();

    explicit overload(Fs... fs) : overload{ std::tuple{ fs... }, partition_t<std::is_empty, Fs...>{} } {}

    template<typename... Gs>
    explicit overload(std::tuple<Fs...> args, typelist<Gs...>) : base{ std::get<Gs>(args)... } {}
};
#else
template<typename... Fs>
struct overload : Fs... {
    using Fs::operator()...;
}
#endif

} // namespace pigro
