#include "../src/pigro/pack_algorithms.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

#include <tuple>

using namespace boost::ut;
using namespace std;

namespace pigro::tests {

suite pack_algorithms_tests = [] {
    "enumerate_pack"_test = [] {
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

    "enumerate_tuple"_test = [] {
        expect(
          enumerate_pack([](auto... items) {
              return (decltype(items)::index + ...);
          },
            tuple{ 1, 2, 3, 4, 5 })
          == 10);

        expect(
          enumerate_pack([](auto... items) {
              return (items.value + ...);
          },
            tuple{ 1, 2, 3, 4, 5 })
          == 25);
    };
};

} // namespace pigro::tests
