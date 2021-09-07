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
          == 10_i);

        expect(
          enumerate_pack([](auto... items) {
              // the index can also be used as a constexpr value
              constexpr auto sum = (items.index + ...);
              return sum;
          },
            1,
            2,
            3,
            4,
            5)
          == 10_i);

        expect(
          enumerate_pack([](auto... items) {
              return (items.value + ...);
          },
            1,
            2,
            3,
            4,
            5)
          == 15_i);
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
              return (items.index + ...);
          },
            tuple{ 1, 2, 3, 4, 5 })
          == 10_i);

        expect(
          enumerate_tuple([](auto... items) {
              return (items.value + ...);
          },
            tuple{ 1, 2, 3, 4, 5 })
          == 15_i);
    };

    "enumerate_n"_test = [] {
        expect(
          enumerate_n<5>([](auto... items) {
              return (items.index + ...);
          })
          == 10);

        expect(
          enumerate_n<5>([](auto... items) {
              return (items.value + ...);
          })
          == 10);
    };

    "value_categories"_test = [] {
        const auto tester = [](bool is_lvalue_reference_expected, auto &&value) {
            enumerate_pack([=](auto &&item) {
                expect((std::is_lvalue_reference_v<decltype(item)> == is_lvalue_reference_expected));
            },
              std::forward<decltype(value)>(value));
        };

        tester(false, 0);

        auto x = 0;
        tester(true, x);
    };
};

} // namespace pigro::tests
