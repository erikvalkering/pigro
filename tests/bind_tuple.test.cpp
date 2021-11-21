#include "../src/pigro/bind_tuple.h"
#include "../src/pigro/compressed_tuple.h"
#include "../src/pigro/overload.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

#include <memory>
#include <tuple>
#include <type_traits>

using namespace boost::ut;

namespace pigro::tests {

struct Empty {
    Empty() = default;
    auto operator<=>(const Empty &) const = default;
};

auto expect_that(bool x) {
    if (!x) throw false;
}

auto stateful_test() {
    auto f1 = compressed_tuple{ 1 } >> [](auto arg) { return arg; };
    expect_that(f1() == 1);
    expect_that(!std::is_empty_v<decltype(f1)>);

    auto f2 = compressed_tuple{ 1, 2, 3, 4 } >> [](auto... args) { return (args + ...); };
    expect_that(f2() == 10);
    expect_that(!std::is_empty_v<decltype(f2)>);

    // TODO: test whether no copies are being made
    // TODO: test non-default-constructible types
}

auto remember_state_test() {
    auto x = 0;
    auto f = compressed_tuple<int &>{ x } >> [](auto &x) { return x++; };

    expect_that(f() == 0);
    expect_that(f() == 1);
    expect_that(f() == 2);
}

auto size_test() {
    auto empty = Empty{};

    auto x = 1;

    auto f1 = compressed_tuple{ empty } >> [](Empty) { return 0; };
    expect_that(f1() == 0);
    expect_that(sizeof(f1) == 1);
    expect_that(std::is_empty_v<decltype(f1)>);

    auto f2 = compressed_tuple{ empty } >> [x](Empty) { return x; };
    expect_that(f2() == 1);
    expect_that(sizeof(f2) == sizeof(int));

    auto f3 = compressed_tuple{ x } >> [](int x) { return x; };
    expect_that(f3() == 1);
    expect_that(sizeof(f3) == sizeof(int));

    auto f4 = compressed_tuple{ x } >> [y = x](int x) { return x + y; };
    expect_that(f4() == 2);
    expect_that(sizeof(f4) == sizeof(int) + sizeof(int));
}

suite bind_tuple_tests = [] {
    auto empty = Empty{};

    "stateless"_test = [=] {
        expect(std::is_empty_v<decltype(empty)>);

        auto f1 = [] { return 0; };
        expect(f1() == 0_i);
        expect(std::is_empty_v<decltype(f1)>);

        auto f2 = [=] { return empty; };
        expect(f2() == empty);
        expect(!std::is_empty_v<decltype(f2)>);

        auto f3 = compressed_tuple{ empty } >> [](auto empty) { return empty; };
        expect(f3() == empty);
        expect(std::is_empty_v<decltype(f3)>);

        auto f4 = compressed_tuple{ empty, empty } >> [](auto empty, auto) { return empty; };
        expect(f4() == empty);
        expect(std::is_empty_v<decltype(f4)>);
    };

    "stateful"_test = stateful_test;

    "extra_parameters"_test = [=] {
        auto f1 = compressed_tuple{ empty } >> [](Empty, int) { return 0; };
        expect(f1(0) == 0_i);
        expect(std::is_empty_v<decltype(f1)>);

        auto f2 = compressed_tuple{ empty } >> [](Empty, int, int) mutable { return 0; };
        expect(f2(0, 0) == 0_i);
        expect(std::is_empty_v<decltype(f2)>);
    };

    "mutable"_test = [=] {
        auto f = compressed_tuple{ empty } >> [](Empty) mutable { return 0; };
        expect(that % f() == 0);
        expect(that % std::is_empty_v<decltype(f)>);
    };

    "sfinae_friendly"_test = [=] {
        auto f = overload{
            compressed_tuple{ empty } >> [](Empty, std::nullptr_t) { return 0; },
            [](double) { return 1; },
        };

        expect(f(nullptr) == 0_i);
        expect(f(0.0) == 1_i);
        expect(std::is_empty_v<decltype(f)>);
    };

    "zero_args"_test = [] {
        auto f = compressed_tuple{} >> [] { return 0; };

        expect(f() == 0);
    };

    "remember_state"_test = remember_state_test;

    "size"_test = size_test;
};

auto sum2 = [](int a, int b) { return a + b; };

auto by_value_test() {
    auto inc = bind_back(sum2, 1);

    expect_that(inc(0) == 1);
}

auto perfect_forward_front_test() {
    auto sum = [](std::unique_ptr<int> a, int b) { return *a + b; };
    auto inc = bind_back(sum, 1);

    expect(inc(std::make_unique<int>(0)) == 1_i);
}

auto perfect_forward_back_test() {
    auto sum = [](int a, const std::unique_ptr<int> &b) { return a + *b; };
    auto inc = bind_back(sum, std::make_unique<int>(1));

    expect(inc(0) == 1_i);
}

auto perfect_forward_callable_test() {
    struct move_only_summer {
        move_only_summer() = default;
        move_only_summer(const move_only_summer &) = delete;
        move_only_summer(move_only_summer &&) = default;

        auto operator()(int a, int b) const { return a + b; }
    };

    auto inc = bind_back(move_only_summer{}, 1);

    expect(inc(0) == 1_i);
}

auto lvalue_reference_callable_test() {
    struct unmoveable_summer {
        unmoveable_summer() = default;
        unmoveable_summer(const unmoveable_summer &) = delete;
        unmoveable_summer(unmoveable_summer &&) = delete;

        auto operator()(int a, int b) const { return a + b; }
    };

    auto sum = unmoveable_summer{};
    auto inc = bind_back(sum, 1);

    expect(inc(0) == 1_i);
}

auto sfinae_friendliness_test() {
    auto lift = [](auto f) { return [=](auto... args) { return f(args...); }; };

    auto f = overload{
        lift([](nullptr_t) { return 1; }),
        bind_back([](nullptr_t, int) { return 1; }, 0),
        [](double) { return 2; },
    };

    expect(f(nullptr) == 1_i);
    expect(f(1.0) == 2_i);
}

suite bind_back_tests = [] {
    "by_value"_test = by_value_test;

    "perfect_forward_front"_test = perfect_forward_front_test;

    "perfect_forward_back"_test = perfect_forward_back_test;

    "perfect_forward_callable"_test = perfect_forward_callable_test;

    "lvalue_reference_callable"_test = lvalue_reference_callable_test;

    "SFINAE-friendliness"_test = sfinae_friendliness_test;
};

suite bind_back_tuple_tests = [] {
    "bind_back_tuple"_test = [] {
        auto f = compressed_tuple{ 1 } << [](auto x, auto y) { return (x - y); };

        expect(f(3) == 2_i);
    };

    "lambda"_test = [] {
        auto f = [] { return 1; };
        auto g = overload{
            compressed_tuple{ f } << [](const int &x, auto f) { return f() + x; },
            [](const double &) { return 4; },
        };

        expect(g(2) == 3_i);
        expect(g(2.0) == 4_i);
    };
};

} // namespace pigro::tests
