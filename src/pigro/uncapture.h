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

template<typename T, size_t tag = 0>
struct Uncaptured {
    T value;

    auto get_value() const { return value; }
};

template<empty T, size_t tag>
struct Uncaptured<T, tag> {
    Uncaptured(T) {}

    auto get_value() const { return empty_object<T>::get(); }
};

template<typename U, typename F, size_t tag>
struct CompressedInvocable : private U
  , F {
    constexpr static auto unique_tag = tag;

    auto operator()(auto &&...args) const {
        return F::operator()(std::forward<decltype(args)>(args)..., U::get_value());
    }

    CompressedInvocable(U u, F f) : U{ u }, F{ f } {};
};

template<typename F>
struct unique_tag { constexpr static auto value = 0; };

template<typename U, typename F, size_t tag>
struct unique_tag<CompressedInvocable<U, F, tag>> {
    constexpr static auto value = tag;
};

template<typename F>
constexpr auto unique_tag_v = unique_tag<F>::value;

template<typename T, typename F>
auto operator>>(Uncaptured<T> u, F f) {
    constexpr auto unique_tag = unique_tag_v<F> + 1;
    return CompressedInvocable<Uncaptured<T, unique_tag>, F, unique_tag>{ Uncaptured<T, unique_tag>{ u.get_value() }, f };
}

} // namespace pigro::detail

namespace pigro {

auto uncaptured(auto value) {
    return detail::Uncaptured<decltype(value)>{ value };
}

} // namespace pigro
