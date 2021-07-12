#pragma once

#include <type_traits>

namespace pigro::detail {

template<typename T>
struct empty_object {
    struct T1 {
        char c;
    };
    struct T2 : T {
        char c;
    };

    union Storage {
        constexpr Storage() : t1{} {}
        T1 t1;
        T2 t2;
    };

    static T get() {
        Storage storage{};

        const auto *c = &storage.t2.c;

        auto t2 = reinterpret_cast<const T2 *>(c);
        auto t = static_cast<const T *>(t2);

        return *t;
    }
};

template<typename T>
concept empty = std::is_empty_v<T>;

template<typename T>
struct Uncaptured {
    T value;

    auto get_value() const { return value; }
};

template<empty T>
struct Uncaptured<T> {
    Uncaptured(T) {}

    auto get_value() const { return empty_object<T>::get(); }
};

template<typename U, typename F>
struct CompressedInvocable : private U
  , F {
    auto operator()(auto &&...args) const {
        return F::operator()(std::forward<decltype(args)>(args)..., U::get_value());
    }

    CompressedInvocable(U u, F f) : U{ u }, F{ f } {};
};

template<typename T, typename F>
auto operator>>(Uncaptured<T> u, F f) {
    return CompressedInvocable<Uncaptured<T>, F>{ u, f };
}

} // namespace pigro::detail

namespace pigro {

auto uncaptured(auto value) {
    return detail::Uncaptured<decltype(value)>{ value };
}

} // namespace pigro
