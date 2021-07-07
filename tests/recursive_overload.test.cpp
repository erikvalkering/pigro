#include "../src/pigro/overload.h"
#include "../src/pigro/recursive.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

using namespace boost::ut;
using namespace std;

namespace pigro::tests {

suite recursive_overload_tests = [] {
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

        const auto g = [] { return 0; };
        const auto h = recursive{ overload{
          [=](auto self) { return g(); },
        } };

        expect(h() == 0_i);

        auto k = recursive{ overload{
          [](auto self) mutable { return 0; },
        } };

        expect(k() == 0_i);
    };
};

} // namespace pigro::tests
