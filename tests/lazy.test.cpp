#include "../src/pigro/lazy.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

#include <type_traits>

using namespace boost::ut;
using namespace std;

namespace pigro::tests {

template<typename T>
struct Spy {
    T object;

    int *comparisons;
    auto operator==(const Spy &rhs) const {
        ++*comparisons;
        return object == rhs.object;
    }
    auto operator!=(const Spy &rhs) const {
        return !this->operator==(rhs);
    }
};

suite lazy_tests = [] {
    "cached"_test = [] {
        auto counter = 0;
        auto foo = lazy([&] {
            ++counter;
            return 42;
        });

        expect(counter == 0_i);
        expect(foo() == 42_i);
        expect(counter == 1_i);

        expect(foo() == 42_i);
        expect(counter == 1_i);
    };

    "dependencies"_test = [] {
        auto bar_counter = 0;
        auto bar_result = 40;
        const auto bar = [&] {
            ++bar_counter;
            return bar_result;
        };

        auto foo_counter = 0;
        auto foo = lazy([&](auto bar) {
            ++foo_counter;
            return bar + 2;
        },
          bar);

        expect(foo_counter == 0_i);
        expect(bar_counter == 0_i);

        expect(foo() == 42_i);
        expect(foo_counter == 1_i);
        expect(bar_counter == 1_i);

        expect(foo() == 42_i);
        expect(foo_counter == 1_i);
        expect(bar_counter == 2_i);

        ++bar_result;
        expect(foo() == 43_i);
        expect(foo_counter == 2_i);
        expect(bar_counter == 3_i);
    };

    "lazy_dependencies"_test = [] {
        auto baz_counter = 0;
        auto baz_result = 0;
        const auto baz = [&] {
            ++baz_counter;
            return baz_result;
        };

        auto bar_counter = 0;
        auto bar = lazy([&](auto baz) {
            ++bar_counter;
            return baz + 2;
        },
          baz);

        auto foo_counter = 0;
        auto foo = lazy([&](auto bar) {
            ++foo_counter;
            return bar + 40;
        },
          bar);

        expect(foo_counter == 0_i);
        expect(bar_counter == 0_i);
        expect(baz_counter == 0_i);
        expect(foo() == 42_i);
        expect(foo_counter == 1_i);
        expect(bar_counter == 1_i);
        expect(baz_counter == 1_i);

        expect(foo() == 42_i);
        expect(foo_counter == 1_i);
        expect(bar_counter == 1_i);
        expect(baz_counter == 2_i);

        ++baz_result;
        expect(foo() == 43_i);
        expect(foo_counter == 2_i);
        expect(bar_counter == 2_i);
        expect(baz_counter == 3_i);

        expect(foo() == 43_i);
        expect(foo_counter == 2_i);
        expect(bar_counter == 2_i);
        expect(baz_counter == 4_i);
    };

    "values"_test = [] {
        auto eval_count = 0;
        auto f = lazy([&](auto x) {
            ++eval_count;
            return x + 40;
        },
          2);

        expect(f() == 42_i);
        expect(f() == 42_i);
        expect(eval_count == 1_i);
    };

    "void"_test = [] {
        auto f = lazy([] {});
        f();

        expect(constant<sizeof(f) == sizeof(bool)>);
        expect(constant<type<decltype(f())> == type<void>>);
    };

    "comparisons"_test = [] {
        auto f_comparisons = 0;
        auto f_result = Spy{ 0, &f_comparisons };
        auto f = [&]() {
            return f_result;
        };

        auto g_comparisons = 0;
        auto g_result = Spy{ 0, &g_comparisons };
        auto g = lazy([&](auto f) {
            return g_result;
        },
          f);

        auto h = lazy([&](auto g) {
            return 0;
        },
          g);

        h();
        expect(*f_result.comparisons == 0_i);
        expect(*g_result.comparisons == 0_i);

        h();
        expect(*f_result.comparisons == 1_i);
        expect(*g_result.comparisons == 0_i);

        ++f_result.object;
        h();
        expect(*f_result.comparisons == 2_i);
        expect(*g_result.comparisons == 1_i);
    };

    "variadic_dependencies"_test = [] {
        const auto sum = [](auto... xs) {
            return (xs + ... + 0);
        };

        auto f = lazy(sum);
        auto g = lazy(sum, 1);
        auto h = lazy(sum, 1, 2);

        expect(f() == 0);
        expect(g() == 1);
        expect(h() == 3);
    };

    "memory_footprint"_test = [] {
        auto f1 = lazy([] {});
        expect(constant<sizeof(f1) == sizeof(bool)>);

        auto f2 = lazy([] { return 0; });
        expect(constant<sizeof(f2) == sizeof(std::optional<int>)>);

        auto x1 = 0;
        auto f3 = lazy([=] { return x1; });
        expect(constant<sizeof(f3) == sizeof(int) + sizeof(std::optional<int>)>);

        auto x2 = 0;
        auto f4 = lazy([&] { return x2; });
        expect(constant<sizeof(f4) == sizeof(int &) + sizeof(std::optional<int>)>);

        auto x3 = 0;
        auto f5 = lazy([](auto x3) { return x3; }, x3);
        expect(constant<sizeof(f5) == sizeof(std::optional<int>) + sizeof(int)>);

        auto x4 = std::integral_constant<int, 0>{};
        auto f6 = lazy([](auto x4) { return x4; }, x4);
        expect(constant<sizeof(f6) == sizeof(std::optional<int>)>);

        auto g1 = [] { return 0; };
        auto f7 = lazy([](auto g1) { return g1; }, g1);
        expect(constant<sizeof(f7) == sizeof(std::optional<int>) + sizeof(std::optional<int>)>);

        auto x5 = 0;
        auto g2 = [=] { return x5; };
        auto f8 = lazy([](auto g2) { return g2; }, g2);
        expect(constant<sizeof(f8) == sizeof(std::optional<int>) + sizeof(std::optional<int>) + sizeof(int)>);

        auto x6 = 0;
        auto g3 = [&] { return x6; };
        auto f9 = lazy([](auto g3) { return g3; }, g3);
        expect(constant<sizeof(f9) == sizeof(std::optional<int>) + sizeof(std::optional<int>) + sizeof(int &)>);

        auto g4 = lazy([] { return 0; });
        auto f10 = lazy([](auto g4) { return g4; }, g4);
        expect(constant<sizeof(f9) == sizeof(std::optional<int>) + sizeof(std::optional<int>)>);
    };
};

} // namespace pigro::tests
