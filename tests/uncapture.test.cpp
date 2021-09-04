#include "../src/pigro/overload.h"
#include "../src/pigro/uncapture.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

#include <type_traits>

using namespace boost::ut;

namespace pigro::tests {

struct Empty {
    auto operator<=>(const Empty &) const = default;
};

suite uncapture_tests = [] {
    auto empty = Empty{};

    "sfinae_friendly"_test = [=] {
        auto f = overload{
            uncaptured(empty) >> [](std::nullptr_t, Empty) { return 0; },
            [](int) { return 1; },
        };

        expect(f(nullptr) == 0_i);
        expect(f(0) == 1_i);
        expect(constant<std::is_empty_v<decltype(f)>>);
    };

};

} // namespace pigro::tests
