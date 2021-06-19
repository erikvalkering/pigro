#include "../src/pigro/recursive.h"

#include <cassert>
#include <iostream>

using namespace std;

namespace pigro::tests {

auto test_recursive = [] {
    cout << "test_recursive" << endl;

    auto fibonacci = recursive{
        [](auto self, int n) -> int {
            if (n < 2) return 1;
            return self(n - 2) + self(n - 1);
        }
    };

    assert(fibonacci(0) == 1);
    assert(fibonacci(1) == 1);
    assert(fibonacci(2) == 2);
    assert(fibonacci(3) == 3);
    assert(fibonacci(4) == 5);

    static_assert(sizeof(fibonacci) == sizeof(char));
    static_assert(std::is_empty_v<decltype(fibonacci)>);

    return 0;
}();

} // namespace pigro::tests
