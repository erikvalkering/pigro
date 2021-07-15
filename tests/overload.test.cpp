#include "../src/pigro/overload.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

using namespace boost::ut;
using namespace std;

namespace pigro::tests {

suite overload_tests = [] {
    "overload"_test = [] {
        const auto f = overload{
            [](int) { return "int"s; },
            [](double) { return "double"s; },
            [](auto) { return "auto"s; },
        };

        expect(f(0) == "int"s);
        expect(f(0.0) == "double"s);
        expect(f(true) == "auto"s);
    };

    "ebo_msvc_workaround"_test = [] {
        auto r1 = overload{
            [] {},
            [] {},
        };
        auto r2 = overload{
            [] {},
            [x = 1] { return x; },
        };
        auto r3 = overload{
            [x = 'a'] { return x; },
            [] {},
        };
        auto r4 = overload{
            [] {},
            [x = 1] { return x; },
            [] {},
            [x = 1] { return x; },
        };
        auto r5 = overload{
            [x = 1] { return x; },
            [] {},
            [x = 1] { return x; },
            [] {},
        };

        static_assert(is_empty_v<decltype(r1)>);
        static_assert(sizeof(r1) == sizeof(char));
        static_assert(!is_empty_v<decltype(r2)>);
        static_assert(sizeof(r2) == sizeof(int));
        static_assert(!is_empty_v<decltype(r3)>);
        static_assert(sizeof(r3) == sizeof(char));
        static_assert(!is_empty_v<decltype(r4)>);
        static_assert(sizeof(r4) == 2 * sizeof(int));
        static_assert(!is_empty_v<decltype(r5)>);
        static_assert(sizeof(r5) == 2 * sizeof(int));
    };

    "empty"_test = [] {
        auto f = overload{};
    };
};

} // namespace pigro::tests
