#pragma once

namespace pigro {

template<typename T>
struct type_ {
    using type = T;
};

template<typename T>
constexpr auto type_c = type_<T>{};

template<typename...>
struct typelist {
    constexpr auto operator==(typelist) const {
        return true;
    }
};

template<
  typename... Ts,
  typename... Us>
constexpr auto operator+(
  typelist<Ts...>,
  typelist<Us...>) {
    return typelist<Ts..., Us...>{};
}

template<typename... TypeLists>
constexpr auto concat(TypeLists... typelists) {
    return (typelists + ... + typelist<>{});
}

template<
  typename T,
  typename Predicate>
constexpr auto filter_if(
  typelist<T>,
  Predicate predicate) {
    if constexpr (predicate(type_c<T>)) {
        return typelist<T>{};
    } else {
        return typelist<>{};
    }
}

template<
  typename... Ts,
  typename Predicate>
constexpr auto filter_if(
  typelist<Ts...>,
  Predicate predicate) {
    return concat(
      filter_if(typelist<Ts>{}, predicate)...);
}

template<
  typename... Ts,
  typename Predicate>
constexpr auto partition(
  typelist<Ts...>,
  Predicate predicate) {
    return concat(
      filter_if(typelist<Ts>{}, predicate)...,
      filter_if(typelist<Ts>{}, !predicate)...);
}

template<
  template<typename>
  class Trait,
  auto is_negated = false>
struct Predicate {
    template<typename T>
    constexpr auto operator()(type_<T>) const {
        return is_negated ? !Trait<T>::value
                          : Trait<T>::value;
    }

    constexpr auto operator!() const {
        return Predicate<Trait, !is_negated>{};
    }
};

template<template<typename> class Trait>
constexpr auto predicate = Predicate<Trait>{};

template<template<typename> class Trait, typename... T>
using partition_t = decltype(partition(typelist<T...>{}, predicate<Trait>));

} // namespace pigro
