#include "../src/pigro/lazy.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

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
};

} // namespace pigro::tests
