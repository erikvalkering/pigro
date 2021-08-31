#include "../src/pigro/bind_tuple.h"
#include "../src/pigro/compressed_tuple.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

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

    "basic"_test = [=] {
        expect(constant<std::is_empty_v<decltype(empty)>>);

        auto f1 = [] { return 0; };
        expect(f1() == 0_i);
        expect(constant<std::is_empty_v<decltype(f1)>>);

        auto f2 = [=] { return empty; };
        expect(f2() == empty);
        expect(constant<!std::is_empty_v<decltype(f2)>>);

        auto f3 = compressed_tuple{ empty } >> [](auto empty) { return empty; };
        expect(f3() == empty);
        expect(constant<std::is_empty_v<decltype(f3)>>);

        // TODO: test whether no copies are being made
        // TODO: test non-default-constructible types
    };
};

} // namespace pigro::tests