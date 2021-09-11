#include "../src/pigro/compressed_tuple.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

#include <tuple>
#include <type_traits>

using namespace boost::ut;

namespace pigro::tests {

struct Empty {
    Empty() = default;
    auto operator<=>(const Empty &) const = default;
};

suite compressed_tuple_tests = [] {
    auto empty = Empty{};

    "basic"_test = [=] {
        auto t1 = compressed_tuple{};
        expect(constant<sizeof(t1) == sizeof(empty)>);
        expect(constant<std::is_empty_v<decltype(t1)>>);

        auto x = int{ 1 };
        auto t2 = compressed_tuple{ x };

        expect(std::get<0>(t2) == x);
        expect(constant<sizeof(t2) == sizeof(int)>);
        expect(constant<!std::is_empty_v<decltype(t2)>>);

        auto y = double{ 2.0 };
        auto t3 = compressed_tuple{ x, y };

        expect(std::get<0>(t3) == x);
        expect(std::get<1>(t3) == y);
        expect(constant<sizeof(t3) == sizeof(double) + sizeof(double)>);
        expect(constant<!std::is_empty_v<decltype(t3)>>);

        auto t4 = compressed_tuple{ empty };

        expect(std::get<0>(t4) == empty);
        expect(constant<sizeof(t4) == sizeof(empty)>);
        expect(constant<std::is_empty_v<decltype(t4)>>);

        auto t5 = compressed_tuple{ empty, y };

        expect(std::get<0>(t5) == empty);
        expect(std::get<1>(t5) == y);
        expect(constant<sizeof(t5) == 0 + sizeof(double)>);
        expect(constant<!std::is_empty_v<decltype(t5)>>);

        auto t6 = compressed_tuple{ y, empty };

        expect(std::get<0>(t6) == y);
        expect(std::get<1>(t6) == empty);
        expect(constant<sizeof(t6) == sizeof(double) + 0>);
        expect(constant<!std::is_empty_v<decltype(t6)>>);
    };

    "compressed_tuple_element_stateful"_test = [] {
        auto e1 = compressed_tuple_element<0>(0);
        expect(e1(e1, idx<0>) == 0);
        expect(type<decltype(e1(e1, idx<0>))> == type<int &>);

        const auto e2 = compressed_tuple_element<0>(0);
        expect(e2(e2, idx<0>) == 0);
        expect(type<decltype(e2(e2, idx<0>))> == type<const int &>);

        auto x1 = 0;
        auto e3 = compressed_tuple_element<0>(x1);
        expect(e3(e3, idx<0>) == x1);
        expect(type<decltype(e3(e3, idx<0>))> == type<int &>);

        const auto x2 = 0;
        auto e4 = compressed_tuple_element<0>(x2);
        expect(e4(e4, idx<0>) == x2);
        expect(type<decltype(e4(e4, idx<0>))> == type<const int &>);

        auto x3 = 0;
        const auto e5 = compressed_tuple_element<0>(x3);
        expect(e5(e5, idx<0>) == x3);
        expect(type<decltype(e5(e5, idx<0>))> == type<const int &>);

        const auto x4 = 0;
        const auto e6 = compressed_tuple_element<0>(x4);
        expect(e6(e6, idx<0>) == x4);
        expect(type<decltype(e6(e6, idx<0>))> == type<const int &>);
    };

    "compressed_tuple_element_stateless"_test = [] {
        auto e1 = compressed_tuple_element<0>(idx_t<0>{});
        expect(e1(e1, idx<0>) == idx<0>);
        expect(type<decltype(e1(e1, idx<0>))> == type<idx_t<0>>);

        const auto e2 = compressed_tuple_element<0>(idx_t<0>{});
        expect(e2(e2, idx<0>) == idx<0>);
        expect(type<decltype(e2(e2, idx<0>))> == type<idx_t<0>>);

        auto x1 = idx<0>;
        auto e3 = compressed_tuple_element<0>(x1);
        expect(e3(e3, idx<0>) == x1);
        expect(type<decltype(e3(e3, idx<0>))> == type<idx_t<0> &>);

        const auto x2 = idx<0>;
        auto e4 = compressed_tuple_element<0>(x2);
        expect(e4(e4, idx<0>) == x2);
        expect(type<decltype(e4(e4, idx<0>))> == type<const idx_t<0> &>);

        auto x3 = idx<0>;
        const auto e5 = compressed_tuple_element<0>(x3);
        expect(e5(e5, idx<0>) == x3);
        expect(type<decltype(e5(e5, idx<0>))> == type<const idx_t<0> &>);

        const auto x4 = idx<0>;
        const auto e6 = compressed_tuple_element<0>(x4);
        expect(e6(e6, idx<0>) == x4);
        expect(type<decltype(e6(e6, idx<0>))> == type<const idx_t<0> &>);
    };

    "SFINAE_friendliness"_test = [] {
        auto f = recursive(overload{
          compressed_tuple_element<0>(0),
          [](auto...) { return 1; },
        });

        expect(f(idx_t<0>{}) == 0_i);
        expect(f(0) == 1_i);

        const auto g = f;
        expect(g(idx_t<0>{}) == 0_i);
        expect(g(0) == 1_i);
    };

    "lvalue_reference"_test = [] {
        auto x = 0;
        auto t = compressed_tuple{ x };
        ++x;

        expect(std::get<0>(t) == x);
    };

    "rvalue_reference"_test = [] {
        auto t = compressed_tuple{ 0 };
        expect(std::get<0>(t) == 0);
    };

    "CTAD"_test = [] {
        const auto t1 = compressed_tuple{};
        expect(type<>(t1) == type<compressed_tuple<>>);

        const auto t2 = compressed_tuple{ 1 };
        expect(type<>(t2) == type<compressed_tuple<int>>);

        auto x = 0;
        const auto t3 = compressed_tuple{ x };
        expect(type<>(t3) == type<compressed_tuple<int &>>);

        const auto y = 0;
        const auto t4 = compressed_tuple{ y };
        expect(type<>(t4) == type<compressed_tuple<const int &>>);
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
    };
};

} // namespace pigro::tests
