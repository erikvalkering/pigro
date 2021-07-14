#include "../src/pigro/regular_void.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

using namespace boost::ut;

namespace pigro::tests {

suite regular_void_tests = [] {
    "regular_void"_test = [] {
        const auto void_a = regular_void{};
        const auto void_b = regular_void{};

        expect(constant<std::regular<regular_void>>);

        expect(void_a == void_a);
        expect(void_a == void_b);
        expect(!(void_a != void_a));
        expect(!(void_a != void_b));
        expect(void_a <= void_a);
        expect(void_a <= void_b);
        expect(void_a >= void_a);
        expect(void_a >= void_b);
        expect(!(void_a < void_a));
        expect(!(void_a < void_b));
        expect(!(void_a > void_a));
        expect(!(void_a > void_b));
    };

    "optional<regular_void>"_test = [] {
        const auto void_a = regular_void{};
        const auto void_b = std::optional<regular_void>{};
        const auto void_c = std::optional<regular_void>{ regular_void{} };

        expect(void_a == void_a);
        expect(void_a != void_b);
        expect(void_a == void_c);
        expect(void_b != void_c);
    };
};

} // namespace pigro::tests
