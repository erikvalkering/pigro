#include "../src/pigro/overload.h"
#include "../src/pigro/recursive.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

using namespace boost::ut;
using namespace std;

namespace pigro::tests {

struct foo {};
struct bar {};

suite recursive_tests = [] {
    "recursive"_test = [] {
        auto fibonacci = recursive{
            [](auto self, int n) -> int {
                if (n < 2) return 1;
                return self(n - 2) + self(n - 1);
            }
        };

        expect(fibonacci(0) == 1_i);
        expect(fibonacci(1) == 1_i);
        expect(fibonacci(2) == 2_i);
        expect(fibonacci(3) == 3_i);
        expect(fibonacci(4) == 5_i);

        static_assert(sizeof(fibonacci) == sizeof(char));
        static_assert(std::is_empty_v<decltype(fibonacci)>);
    };

    "recursive_overload"_test = [] {
        const auto f = recursive{ overload{
          [](auto, int) { return "int"s; },
          [](auto, double) { return "double"s; },
          [](auto, auto) { return "auto"s; },
          [](auto self, string) { return self(0); },
        } };

        expect(f(0) == "int"s);
        expect(f(0.0) == "double"s);
        expect(f(true) == "auto"s);
        expect(f("a"s) == "int"s);
    };

    "state"_test = [] {
        const auto g = [] { return 0; };
        const auto h = recursive{ overload{
          [=](auto) { return g(); },
        } };

        expect(h() == 0_i);
    };

    "mutable"_test = [] {
        auto k = recursive{ overload{
          [](auto) mutable { return 0; },
        } };

        expect(k() == 0_i);
    };

    "SFINAE_friendliness"_test = [] {
        auto f = recursive{
            overload{
              []<size_t idx>(auto, idx_t<idx>) -> std::enable_if_t<idx <= 1, size_t> {
                  return idx;
              },
              [](auto...) { return 2; },
            }
        };

        expect(f(idx_t<0>{}) == 0_i);
        expect(f(idx_t<1>{}) == 1_i);
        expect(f(0) == 2_i);

        const auto g = f;
        expect(g(idx_t<0>{}) == 0_i);
        expect(g(idx_t<1>{}) == 1_i);
        expect(g(0) == 2_i);
    };

    "empty_recursive_overload"_test = [] {
        recursive{ overload{} };
    };

    "default_construction_stateless"_test = [] {
        const auto f = recursive{ [](auto) {} };
        const auto g = decltype(f){};
    };

    "value_categories"_test = [] {
        auto f = recursive{
            [](auto &&self) -> decltype(auto) { return std::forward<decltype(self)>(self); }
        };

        expect(type<decltype(f())> == type<decltype(f) &>);
        expect(type<decltype(std::move(f)())> == type<decltype(f) &&>);

        const auto cf = f;
        expect(type<decltype(cf())> == type<const decltype(f) &>);
        expect(type<decltype(std::move(cf)())> == type<const decltype(f) &&>);
    };

    "perfect_forwarding"_test = [] {
        struct Foo {
            auto operator()(auto &&) & { return 1; }
            auto operator()(auto &&) && { return 2; }
            auto operator()(auto &&) const & { return 3; }
            auto operator()(auto &&) const && { return 4; }
        };

        auto f = recursive{ Foo{} };

        expect(f() == 1);
        expect(std::move(f)() == 2);

        const auto cf = f;
        expect(cf() == 3);
        expect(std::move(cf)() == 4);
    };
};

} // namespace pigro::tests
