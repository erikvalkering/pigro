#include "../src/pigro/pack_algorithms.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

using namespace boost::ut;
using namespace std;

namespace pigro::tests {

suite pack_algorithms_tests = [] {
    "basic"_test = [] {
        expect(
          enumerate_pack([](auto... items) {
              return (decltype(items)::index + ...);
          },
            1,
            2,
            3,
            4,
            5)
          == 10);

        expect(
          enumerate_pack([](auto... items) {
              return (items.value + ...);
          },
            1,
            2,
            3,
            4,
            5)
          == 25);
    };
};

} // namespace pigro::tests
