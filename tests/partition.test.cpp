#include "../src/pigro/partition.h"

#include <iostream>
#include <type_traits>

using namespace std;

namespace pigro::tests {

auto test_partition = [] {
    cout << "test_partition" << endl;

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

    return 0;
}();

} // namespace pigro::tests
