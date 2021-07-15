#include "../src/pigro/partition.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

#include <type_traits>

using namespace boost::ut;
using namespace std;

namespace pigro::tests {

suite partition_tests = [] {
    "partition"_test = [] {
        struct Empty {};

        constexpr auto list = typelist<
          int,
          char,
          Empty,
          float,
          Empty,
          int>{};

        static_assert(
          filter_if(list, predicate<std::is_empty>)
          == typelist<Empty, Empty>{});
        static_assert(
          filter_if(list, !predicate<std::is_empty>)
          == typelist<int, char, float, int>{});
        static_assert(
          partition(list, predicate<std::is_empty>)
          == typelist<
            Empty,
            Empty,
            int,
            char,
            float,
            int>{});
    };

    "empty"_test = [] {
        expect(partition(typelist<>{}, predicate<std::is_empty>) == typelist<>{});
    };
};

} // namespace pigro::tests
