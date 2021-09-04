#include "../src/pigro/bind_tuple.h"
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

suite bind_tuple_tests = [] {
    auto empty = Empty{};

    "stateless"_test = [=] {
        expect(constant<std::is_empty_v<decltype(empty)>>);

        auto f1 = [] { return 0; };
        expect(f1() == 0_i);
        expect(constant<std::is_empty_v<decltype(f1)>>);

        auto f2 = [=] { return empty; };
        expect(f2() == empty);
        expect(constant<!std::is_empty_v<decltype(f2)>>);

        auto f3 = compressed_tuple{ empty } >> [](auto empty) { return empty; };
        expect(f3() == empty);
        expect(constant<std::is_empty_v<decltype(f3)>>);

        auto f4 = compressed_tuple{ empty, empty } >> [](auto empty, auto) { return empty; };
        expect(f4() == empty);
        expect(constant<std::is_empty_v<decltype(f4)>>);
    };

    "stateful"_test = [=] {
        auto f1 = compressed_tuple{ 1 } >> [](auto arg) { return arg; };
        expect(f1() == 1);
        expect(constant<!std::is_empty_v<decltype(f1)>>);

        auto f2 = compressed_tuple{ 1, 2, 3, 4 } >> [](auto... args) { return (args + ...); };
        expect(f2() == 10);
        expect(constant<!std::is_empty_v<decltype(f2)>>);
        // TODO: test whether no copies are being made
        // TODO: test non-default-constructible types
    };

    "extra_parameters"_test = [=] {
        auto f1 = compressed_tuple{ empty } >> [](Empty, int) { return 0; };
        expect(f1(0) == 0_i);
        expect(constant<std::is_empty_v<decltype(f1)>>);

        auto f2 = compressed_tuple{ empty } >> [](Empty, int, int) mutable { return 0; };
        expect(f2(0, 0) == 0_i);
        expect(constant<std::is_empty_v<decltype(f2)>>);
    };

    "mutable"_test = [=] {
        auto f = compressed_tuple{ empty } >> [](Empty) mutable { return 0; };
        expect(f() == 0_i);
        expect(constant<std::is_empty_v<decltype(f)>>);
    };

    "zero_args"_test = [] {
        auto f = compressed_tuple{} >> [] { return 0; };

        expect(f() == 0_i);
    };

    "remember_state"_test = [=] {
        auto x = 0;
        auto f = compressed_tuple{ x } >> [](auto &&x) mutable { return x++; };

        expect(f() == 0_i);
        expect(f() == 1_i);
        expect(f() == 2_i);
    };

    "size"_test = [=] {
        auto x = 1;

        auto f1 = compressed_tuple{ empty } >> [](Empty) { return 0; };
        expect(f1() == 0_i);
        expect(constant<sizeof(f1) == 1_i>);
        expect(constant<std::is_empty_v<decltype(f1)>>);

        auto f2 = compressed_tuple{ empty } >> [x](Empty) { return x; };
        expect(f2() == 1_i);
        expect(constant<sizeof(f2) == sizeof(int)>);

        auto f3 = compressed_tuple{ x } >> [](int x) { return x; };
        expect(f3() == 1_i);
        expect(constant<sizeof(f3) == sizeof(int)>);

        auto f4 = compressed_tuple{ x } >> [y = x](int x) { return x + y; };
        expect(f4() == 2_i);
        expect(constant<sizeof(f4) == sizeof(int) + sizeof(int)>);
    };
};

} // namespace pigro::tests
