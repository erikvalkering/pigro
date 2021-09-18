#include "../src/pigro/apply.h"
#include "../src/pigro/compressed_tuple.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

#include <tuple>

using namespace boost::ut;

namespace pigro::tests {

suite apply_tests = [] {
    "apply_t"_test = [] {
        auto f = [](int, int, int) { return 4; };

        auto t1 = std::tuple{ 1, 2, 3 };
        expect(std::apply(f, t1) == 4_i);
        expect(pigro::apply(f, t1) == 4_i);

        auto t2 = pigro::compressed_tuple{ 1, 2, 3 };
        expect(pigro::apply(f, t2) == 4_i);
    };
};

} // namespace pigro::tests
