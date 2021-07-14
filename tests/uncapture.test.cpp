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

    "remember_state"_test = [=] {
        auto x = 0;
        auto f2 = uncaptured(x) >> [](auto &&x) { return x++; };

        expect(f2() == 0_i);
        expect(f2() == 1_i);
        expect(f2() == 2_i);
    };
};

} // namespace pigro::tests
