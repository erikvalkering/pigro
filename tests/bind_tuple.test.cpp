#include "../src/pigro/bind_tuple.h"
#include "../src/pigro/compressed_tuple.h"

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

    "stateful"_test = [=] {
        auto f1 = compressed_tuple{ 1 } >> [](auto arg) { return arg; };
        expect(f1() == 1_i);
        expect(!std::is_empty_v<decltype(f1)>);

        auto f2 = compressed_tuple{ 1, 2, 3, 4 } >> [](auto... args) { return (args + ...); };
        expect(f2() == 10_i);
        expect(!std::is_empty_v<decltype(f2)>);
        // TODO: test whether no copies are being made
        // TODO: test non-default-constructible types
    };

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
        expect(f() == 0_i);
        expect(std::is_empty_v<decltype(f)>);
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

        expect(f() == 0_i);
    };

    "remember_state"_test = [=] {
        auto x = 0;
        auto f = compressed_tuple<int &>{ x } >> [](int &x) { x++; };

        expect(f() == 0_i);
        expect(f() == 1_i);
        expect(f() == 2_i);
    };

    "size"_test = [=] {
        auto x = 1;

        auto f1 = compressed_tuple{ empty } >> [](Empty) { return 0; };
        expect(f1() == 0_i);
        expect(sizeof(f1) == 1_i);
        expect(std::is_empty_v<decltype(f1)>);

        auto f2 = compressed_tuple{ empty } >> [x](Empty) { return x; };
        expect(f2() == 1_i);
        expect(sizeof(f2) == sizeof(int));

        auto f3 = compressed_tuple{ x } >> [](int x) { return x; };
        expect(f3() == 1_i);
        expect(sizeof(f3) == sizeof(int));

        auto f4 = compressed_tuple{ x } >> [y = x](int x) { return x + y; };
        expect(f4() == 2_i);
        expect(sizeof(f4) == sizeof(int) + sizeof(int));
    };
};

suite bind_back_tests = [] {
    "by_value"_test = [] {
        auto sum = [](int a, int b) { return a + b; };
        auto inc = bind_back(sum, 1);

        expect(inc(0) == 1_i);
    };

    "perfect_forward_front"_test = [] {
        auto sum = [](std::unique_ptr<int> a, int b) { return *a + b; };
        auto inc = bind_back(sum, 1);

        expect(inc(std::make_unique<int>(0)) == 1_i);
    };

    "perfect_forward_back"_test = [] {
        auto sum = [](int a, std::unique_ptr<int> &b) { return a + *b; };
        auto inc = bind_back(sum, std::make_unique<int>(1));

        expect(inc(0) == 1_i);
    };

    "perfect_forward_callable"_test = [] {
        struct move_only_summer {
            move_only_summer() = default;
            move_only_summer(const move_only_summer &) = delete;
            move_only_summer(move_only_summer &&) = default;

            auto operator()(int a, int b) const { return a + b; }
        };

        auto inc = bind_back(move_only_summer{}, 1);

        expect(inc(0) == 1_i);
    };

    "lvalue_reference_callable"_test = [] {
        struct unmoveable_summer {
            unmoveable_summer() = default;
            unmoveable_summer(const unmoveable_summer &) = delete;
            unmoveable_summer(unmoveable_summer &&) = delete;

            auto operator()(int a, int b) const { return a + b; }
        };

        auto sum = unmoveable_summer{};
        auto inc = bind_back(sum, 1);

        expect(inc(0) == 1_i);
    };
};

} // namespace pigro::tests
