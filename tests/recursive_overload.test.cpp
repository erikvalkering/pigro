#include "../src/pigro/recursive_overload.h"

#include <cassert>
#include <iostream>

using namespace std;

namespace pigro::tests {

auto test_recursive_overload = [] {
    cout << "test_recursive_overload" << endl;

    const auto f = recursive_overload{
        [](auto self, int) { return "int"s; },
        [](auto self, double) { return "double"s; },
        [](auto self, auto) { return "auto"s; },
        [](auto self, string) { return self(0); },
    };

    assert(f(0) == "int");
    assert(f(0.0) == "double");
    assert(f(true) == "auto");
    assert(f("a"s) == "int");

    const auto g = [] { return 0; };
    const auto h = recursive_overload{
        [=](auto self) { return g(); },
    };

    assert(h() == 0);

    auto k = recursive_overload{
        [](auto self) mutable { return 0; },
    };

    assert(k() == 0);

    return 0;
}();

} // namespace pigro::tests
