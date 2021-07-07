#include "../src/pigro/recursive.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

using namespace boost::ut;
using namespace std;

namespace pigro::tests {

suite recursive_tests = [] {
    "recursive"_test = [] {
        auto fibonacci = recursive{
            [](auto self, int n) -> int {
                if (n < 2) return 1;
                return self(n - 2) + self(n - 1);
            }
        };

        expect(fibonacci(0) == 1_i);
        expect(fibonacci(1) == 1_i);
        expect(fibonacci(2) == 2_i);
        expect(fibonacci(3) == 3_i);
        expect(fibonacci(4) == 5_i);

        static_assert(sizeof(fibonacci) == sizeof(char));
        static_assert(std::is_empty_v<decltype(fibonacci)>);
    };
};

} // namespace pigro::tests
