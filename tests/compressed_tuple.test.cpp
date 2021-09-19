#include "../src/pigro/compressed_tuple.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

#include <tuple>
#include <type_traits>

using namespace boost::ut;

namespace pigro::tests {

suite compressed_tuple_element_tests = [] {
    "compressed_tuple_element_stateful"_test = [] {
        auto e1 = recursive{ compressed_tuple_element<0>(0) };
        expect(e1(idx<0>) == 0);
        expect(type<decltype(e1(idx<0>))> == type<int &>);

        const auto e2 = recursive{ compressed_tuple_element<0>(0) };
        expect(e2(idx<0>) == 0);
        expect(type<decltype(e2(idx<0>))> == type<const int &>);

        auto x1 = 0;
        auto e3 = recursive{ compressed_tuple_element<0>(x1) };
        expect(e3(idx<0>) == x1);
        expect(type<decltype(e3(idx<0>))> == type<int &>);

        const auto x2 = 0;
        auto e4 = recursive{ compressed_tuple_element<0>(x2) };
        expect(e4(idx<0>) == x2);
        expect(type<decltype(e4(idx<0>))> == type<const int &>);

        auto x3 = 0;
        const auto e5 = recursive{ compressed_tuple_element<0>(x3) };
        expect(e5(idx<0>) == x3);
        expect(type<decltype(e5(idx<0>))> == type<const int &>);

        const auto x4 = 0;
        const auto e6 = recursive{ compressed_tuple_element<0>(x4) };
        expect(e6(idx<0>) == x4);
        expect(type<decltype(e6(idx<0>))> == type<const int &>);
    };

    "compressed_tuple_element_stateless"_test = [] {
        auto e1 = recursive{ compressed_tuple_element<0>(idx_t<0>{}) };
        expect(e1(idx<0>) == idx<0>);
        expect(type<decltype(e1(idx<0>))> == type<idx_t<0>>);

        const auto e2 = recursive{ compressed_tuple_element<0>(idx_t<0>{}) };
        expect(e2(idx<0>) == idx<0>);
        expect(type<decltype(e2(idx<0>))> == type<idx_t<0>>);

        auto x1 = idx<0>;
        auto e3 = recursive{ compressed_tuple_element<0>(x1) };
        expect(e3(idx<0>) == x1);
        expect(type<decltype(e3(idx<0>))> == type<idx_t<0>>);

        const auto x2 = idx<0>;
        auto e4 = recursive{ compressed_tuple_element<0>(x2) };
        expect(e4(idx<0>) == x2);
        expect(type<decltype(e4(idx<0>))> == type<idx_t<0>>);

        auto x3 = idx<0>;
        const auto e5 = recursive{ compressed_tuple_element<0>(x3) };
        expect(e5(idx<0>) == x3);
        expect(type<decltype(e5(idx<0>))> == type<idx_t<0>>);

        const auto x4 = idx<0>;
        const auto e6 = recursive{ compressed_tuple_element<0>(x4) };
        expect(e6(idx<0>) == x4);
        expect(type<decltype(e6(idx<0>))> == type<idx_t<0>>);
    };

    "SFINAE_friendliness"_test = [] {
        auto f = recursive{
            overload{
              compressed_tuple_element<0>(0),
              [](auto...) { return 1; },
            }
        };

        expect(f(idx<0>) == 0_i);
        expect(f(0) == 1_i);

        const auto g = f;
        expect(g(idx<0>) == 0_i);
        expect(g(0) == 1_i);
    };
};

struct Empty {
    Empty() = default;
    auto operator<=>(const Empty &) const = default;
};

suite compressed_tuple_tests = [] {
    auto empty = Empty{};

    "basic"_test = [=] {
        auto t1 = compressed_tuple{};
        expect(sizeof(t1) == sizeof(empty));
        expect(std::is_empty_v<decltype(t1)>);

        auto x = int{ 1 };
        auto t2 = compressed_tuple{ x };

        expect(pigro::get<0>(t2) == x);
        expect(sizeof(t2) == sizeof(int));
        expect(!std::is_empty_v<decltype(t2)>);

        auto y = double{ 2.0 };
        auto t3 = compressed_tuple{ x, y };

        expect(pigro::get<0>(t3) == x);
        expect(pigro::get<1>(t3) == y);
        expect(sizeof(t3) == sizeof(double) + sizeof(double));
        expect(!std::is_empty_v<decltype(t3)>);

        auto t4 = compressed_tuple{ empty };

        expect(pigro::get<0>(t4) == empty);
        expect(sizeof(t4) == sizeof(empty));
        expect(std::is_empty_v<decltype(t4)>);

        auto t5 = compressed_tuple{ empty, y };

        expect(pigro::get<0>(t5) == empty);
        expect(pigro::get<1>(t5) == y);
        expect(sizeof(t5) == 0 + sizeof(double));
        expect(!std::is_empty_v<decltype(t5)>);

        auto t6 = compressed_tuple{ y, empty };

        expect(pigro::get<0>(t6) == y);
        expect(pigro::get<1>(t6) == empty);
        expect(sizeof(t6) == sizeof(double) + 0);
        expect(!std::is_empty_v<decltype(t6)>);
    };

    "lvalue_reference"_test = [] {
        auto x = 0;
        auto t = compressed_tuple<int &>{ x };
        expect(pigro::get<0>(t) == 0);

        ++x;
        expect(pigro::get<0>(t) == 1);

        ++pigro::get<0>(t);
        expect(x == 2);
    };

    "rvalue_reference"_test = [] {
        auto t = compressed_tuple{ 0 };
        expect(pigro::get<0>(t) == 0);

        ++pigro::get<0>(t);
        expect(pigro::get<0>(t) == 1);
    };

    "CTAD"_test = [] {
        const auto t1 = compressed_tuple{};
        expect(type<>(t1) == type<compressed_tuple<>>);

        const auto t2 = compressed_tuple{ 1 };
        expect(type<>(t2) == type<compressed_tuple<int>>);

        auto x = 0;
        const auto t3 = compressed_tuple{ x };
        expect(type<>(t3) == type<compressed_tuple<int>>);

        const auto y = 0;
        const auto t4 = compressed_tuple{ y };
        expect(type<>(t4) == type<compressed_tuple<int>>);
    };

    "default_construction"_test = [] {
        compressed_tuple{};
        compressed_tuple<>{};
        compressed_tuple<int>{};
        compressed_tuple<int, bool>{};

        expect(std::default_initializable<compressed_tuple<>>);
        expect(std::default_initializable<compressed_tuple<int>>);
        expect(std::default_initializable<compressed_tuple<int, bool>>);

        expect(!std::default_initializable<compressed_tuple<int &, bool>>);
        expect(!std::default_initializable<compressed_tuple<int &&, bool>>);
    };

    "tuple_interface"_test = [] {
        expect(concepts::tuple_like<compressed_tuple<>>);
        expect(concepts::tuple_like<compressed_tuple<int>>);
        expect(concepts::tuple_like<compressed_tuple<const int>>);
        expect(concepts::tuple_like<compressed_tuple<int &>>);
        expect(concepts::tuple_like<compressed_tuple<const int &>>);
        expect(concepts::tuple_like<compressed_tuple<int &&>>);
        expect(concepts::tuple_like<compressed_tuple<const int &&>>);
    };

    "pigro::concepts::compressed_tuple"_test = [] {
        expect(pigro::concepts::compressed_tuple<compressed_tuple<>>);
        expect(pigro::concepts::compressed_tuple<compressed_tuple<int>>);
        expect(pigro::concepts::compressed_tuple<compressed_tuple<int, float>>);
        expect(pigro::concepts::compressed_tuple<compressed_tuple<int, int, int>>);

        expect(pigro::concepts::compressed_tuple<compressed_tuple<> &>);
        expect(pigro::concepts::compressed_tuple<compressed_tuple<> &&>);
        expect(pigro::concepts::compressed_tuple<const compressed_tuple<>>);
        expect(pigro::concepts::compressed_tuple<const compressed_tuple<> &>);
        expect(pigro::concepts::compressed_tuple<const compressed_tuple<> &&>);
    };

    "std::tuple_size_v"_test = [] {
        expect(std::tuple_size_v<compressed_tuple<>> == 0_i);
        expect(std::tuple_size_v<compressed_tuple<int>> == 1_i);
        expect(std::tuple_size_v<compressed_tuple<int, float>> == 2_i);
        expect(std::tuple_size_v<compressed_tuple<int, int, int>> == 3_i);

        expect(std::tuple_size_v<compressed_tuple<> &> == 0_i);
        expect(std::tuple_size_v<compressed_tuple<> &&> == 0_i);
        expect(std::tuple_size_v<const compressed_tuple<>> == 0_i);
        expect(std::tuple_size_v<const compressed_tuple<> &> == 0_i);
        expect(std::tuple_size_v<const compressed_tuple<> &&> == 0_i);
    };

    "std::tuple_element_t"_test = [] {
        expect(type<std::tuple_element_t<0, compressed_tuple<int, float, double>>> == type<std::tuple_element_t<0, std::tuple<int, float, double>>>);
        expect(type<std::tuple_element_t<1, compressed_tuple<int, float, double>>> == type<std::tuple_element_t<1, std::tuple<int, float, double>>>);
        expect(type<std::tuple_element_t<2, compressed_tuple<int, float, double>>> == type<std::tuple_element_t<2, std::tuple<int, float, double>>>);

        expect(type<std::tuple_element_t<0, const compressed_tuple<int>>> == type<std::tuple_element_t<0, const std::tuple<int>>>);
    };

    "pigro::get ADL"_test = [] {
        auto t = compressed_tuple{ 0 };

        expect(get<0>(t) == 0_i);
        expect(type<decltype(get<0>(t))> == type<int &>);

        expect(get<0>(static_cast<decltype(t) &&>(t)) == 0_i);
        expect(type<decltype(get<0>(static_cast<decltype(t) &&>(t)))> == type<int &&>);

        expect(get<0>(static_cast<const decltype(t) &>(t)) == 0_i);
        expect(type<decltype(get<0>(static_cast<const decltype(t) &>(t)))> == type<const int &>);

        expect(get<0>(static_cast<const decltype(t) &&>(t)) == 0_i);
        expect(type<decltype(get<0>(static_cast<const decltype(t) &&>(t)))> == type<const int &&>);
    };
};

} // namespace pigro::tests
