#include "../src/pigro/tuple_algorithms.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

#include <functional>

using namespace boost::ut;
using namespace std;

namespace pigro::tests {

suite tuple_algorithms_tests = [] {
    "transform"_test = [] {
        constexpr auto inv = logical_not{};

        expect(transform(tuple{}, inv) == tuple{});

        expect(transform(tuple{ false }, inv) == tuple{ true });
        expect(transform(tuple{ true }, inv) == tuple{ false });

        expect(transform(tuple{ false, false }, inv) == tuple{ true, true });
        expect(transform(tuple{ false, true }, inv) == tuple{ true, false });
        expect(transform(tuple{ true, false }, inv) == tuple{ false, true });
        expect(transform(tuple{ true, true }, inv) == tuple{ false, false });
    };

    "any"_test = [] {
        expect(any(tuple{}) == false);

        expect(any(tuple{ false }) == false);
        expect(any(tuple{ true }) == true);

        expect(any(tuple{ false, false }) == false);
        expect(any(tuple{ false, true }) == true);
        expect(any(tuple{ true, false }) == true);
        expect(any(tuple{ true, true }) == true);
    };

    "any_with_predicate"_test = [] {
        constexpr auto predicate = [](auto x) { return x == "true"s; };

        expect(any(tuple{ "false" }, predicate) == false);
        expect(any(tuple{ "true" }, predicate) == true);

        expect(any(tuple{ "false", "false" }, predicate) == false);
        expect(any(tuple{ "false", "true" }, predicate) == true);
        expect(any(tuple{ "true", "false" }, predicate) == true);
        expect(any(tuple{ "true", "true" }, predicate) == true);
    };
};

} // namespace pigro::tests
