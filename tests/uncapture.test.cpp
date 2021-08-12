#include "../src/pigro/uncapture.h"
#include "../src/pigro/overload.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

using namespace boost::ut;

namespace pigro::tests {

struct Empty {
    auto operator<=>(const Empty &) const = default;
};

suite uncapture_tests = [] {
    auto empty = Empty{};

    "uncapture"_test = [=] {
        expect(constant<std::is_empty_v<decltype(empty)>>);

        auto f1 = [] { return 0; };
        expect(f1() == 0_i);
        expect(constant<std::is_empty_v<decltype(f1)>>);

        auto f2 = [=] { return empty; };
        expect(f2() == empty);
        expect(constant<!std::is_empty_v<decltype(f2)>>);

        auto f3 = uncaptured(empty) >> [](auto empty) { return empty; };
        expect(f2() == empty);
        expect(constant<std::is_empty_v<decltype(f3)>>);
    };

    "extra_parameters"_test = [=] {
        auto f1 = uncaptured(empty) >> [](int, Empty) { return 0; };
        expect(f1(0) == 0_i);
        expect(constant<std::is_empty_v<decltype(f1)>>);

        auto f2 = uncaptured(empty) >> [](int, int, Empty) mutable { return 0; };
        expect(f2(0, 0) == 0_i);
        expect(constant<std::is_empty_v<decltype(f2)>>);
    };

    "mutable"_test = [=] {
        auto f = uncaptured(empty) >> [](Empty) mutable { return 0; };
        expect(f() == 0_i);
        expect(constant<std::is_empty_v<decltype(f)>>);
    };

    "sfinae_friendly"_test = [=] {
        auto f = overload{
            uncaptured(empty) >> [](std::nullptr_t, Empty) { return 0; },
            [](int) { return 1; },
        };

        expect(f(nullptr) == 0_i);
        expect(f(0) == 1_i);
        expect(constant<std::is_empty_v<decltype(f)>>);
    };

    "variadic"_test = [=] {
        auto f1 = uncaptured(empty) >> (uncaptured(empty) >> (uncaptured(empty) >> [](auto...) { return 0; }));
        expect(f1(0) == 0_i);
        expect(constant<std::is_empty_v<decltype(f1)>>);

        auto f2 = uncaptured(1) >> (uncaptured(10) >> (uncaptured(100) >> [](int a, int b, int c) { return c - b - a; }));
        expect(f2() == 100 - 10 - 1);
        expect(constant<!std::is_empty_v<decltype(f2)>>);

        auto f3 = uncaptured(empty, empty, empty) >> [](auto...) { return 0; };
        expect(f3(0) == 0_i);
        expect(constant<std::is_empty_v<decltype(f3)>>);

        auto f4 = uncaptured(1, 10, 100) >> [](int a, int b, int c) { return c - b - a; };
        expect(f4() == 100 - 10 - 1);
        expect(constant<!std::is_empty_v<decltype(f4)>>);
    };

    "zero_args"_test = [] {
        auto f = uncaptured() >> []() { return 0; };

        expect(f() == 0_i);
    };

    "remember_state"_test = [=] {
        auto x = 0;
        auto f = uncaptured(x) >> [](auto &&x) { return x++; };

        expect(f() == 0_i);
        expect(f() == 1_i);
        expect(f() == 2_i);
    };

    "only_uncaptured"_test = [=] {
        auto f1 = uncaptured();
        expect(constant<sizeof(f1) == sizeof(empty)>);
        expect(constant<std::is_empty_v<decltype(f1)>>);

        auto x = int{ 1 };
        auto f2 = uncaptured(x);

        expect(f2(std::integral_constant<size_t, 0>{}) == x);
        expect(constant<sizeof(f2) == sizeof(int)>);
        expect(constant<!std::is_empty_v<decltype(f2)>>);

        auto y = double{ 2.0 };
        auto f3 = uncaptured(x, y);

        expect(f3(std::integral_constant<size_t, 0>{}) == x);
        expect(f3(std::integral_constant<size_t, 1>{}) == y);
        expect(constant<sizeof(f3) == sizeof(double) + sizeof(double)>);
        expect(constant<!std::is_empty_v<decltype(f3)>>);
    };

    "size"_test = [=] {
        auto x = 1;

        auto f1 = uncaptured(empty) >> [](Empty) { return 0; };
        expect(f1() == 0_i);
        expect(constant<sizeof(f1) == 1_i>);
        expect(constant<std::is_empty_v<decltype(f1)>>);

        auto f2 = uncaptured(empty) >> [x](Empty) { return x; };
        expect(f2() == 1_i);
        expect(constant<sizeof(f2) == sizeof(int)>);

        auto f3 = uncaptured(x) >> [](int x) { return x; };
        expect(f3() == 1_i);
        expect(constant<sizeof(f3) == sizeof(int)>);

        auto f4 = uncaptured(x) >> [y = x](int x) { return x + y; };
        expect(f4() == 2_i);
        expect(constant<sizeof(f4) == sizeof(int) + sizeof(int)>);
    };
};

} // namespace pigro::tests
