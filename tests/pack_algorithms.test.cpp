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
          == 15);
    };

    "tuple_should_remain_tuple"_test = [] {
        // Before, the implementation of enumerate_pack
        // would use std::tuple{ pack... } to create the
        // tuple. However, when pack is a single tuple,
        // this would not result in a std::tuple{std::tuple},
        // but simply invoke the copy constructor.
        // As a result, the tuple's elements would be
        // enumerated, instead of the tuple as a whole.
        // In order to enumerate a tuple, you should use
        // enumerate_tuple().
        //
        // This test ensures that when we enumerate over a single
        // tuple, we get back the tuple.
        expect(
          enumerate_pack([](auto... items) {
              return (items.value + ...);
          },
            tuple{ 1, 2, 3, 4, 5 })
          == tuple{ 1, 2, 3, 4, 5 });
    };

    "enumerate_tuple"_test = [] {
        expect(
          enumerate_tuple([](auto... items) {
              return (decltype(items)::index + ...);
          },
            tuple{ 1, 2, 3, 4, 5 })
          == 10);

        expect(
          enumerate_tuple([](auto... items) {
              return (items.value + ...);
          },
            tuple{ 1, 2, 3, 4, 5 })
          == 15);
    };

    "enumerate_n"_test = [] {
        expect(
          enumerate_n<5>([](auto... items) {
              return (decltype(items)::index + ...);
          })
          == 10);

        expect(
          enumerate_n<5>([](auto... items) {
              return (items.value + ...);
          })
          == 10);
    };
};

} // namespace pigro::tests
