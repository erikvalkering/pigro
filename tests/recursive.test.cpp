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
          [](auto self, int) { return "int"s; },
          [](auto self, double) { return "double"s; },
          [](auto self, auto) { return "auto"s; },
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
          [=](auto self) { return g(); },
        } };

        expect(h() == 0_i);
    };

    "mutable"_test = [] {
        auto k = recursive{ overload{
          [](auto self) mutable { return 0; },
        } };

        expect(k() == 0_i);
    };

    "sfinae_friendly"_test = [] {
        auto f =
          overload{
              recursive{
                [](auto self, foo) { return 0; } },
              recursive{
                [](auto self, bar) { return 1; } },
          };

        expect(f(foo{}) == 0_i);
        expect(f(bar{}) == 1_i);
    };

    "default_construction"_test = [] {
        recursive{};
        recursive{ overload{} };
        overload{ recursive{} };

        const auto f = recursive{
            overload{
              [](auto, int) {},
              [](auto, bool) {},
            }
        };

        const auto g = decltype(f){};
    };
};

} // namespace pigro::tests
