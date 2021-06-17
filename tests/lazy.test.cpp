#include "../src/pigro/lazy.h"

#include <cassert>
#include <iostream>

using namespace std;

namespace pigro::tests {

auto test_cached = [] {
    cout << "test_cached" << endl;

    auto counter = 0;
    auto foo = lazy([&] {
        ++counter;
        return 42;
    });

    assert(counter == 0);
    assert(foo() == 42);
    assert(counter == 1);

    assert(foo() == 42);
    assert(counter == 1);

    return 0;
}();

auto test_dependencies = [] {
    cout << "test_dependencies" << endl;

    auto bar_counter = 0;
    auto bar_result = 40;
    const auto bar = [&] {
        ++bar_counter;
        return bar_result;
    };

    auto foo_counter = 0;
    auto foo = lazy([&](auto bar) {
        ++foo_counter;
        return bar + 2;
    },
      bar);

    assert(foo_counter == 0);
    assert(bar_counter == 0);

    assert(foo() == 42);
    assert(foo_counter == 1);
    assert(bar_counter == 1);

    assert(foo() == 42);
    assert(foo_counter == 1);
    assert(bar_counter == 2);

    ++bar_result;
    assert(foo() == 43);
    assert(foo_counter == 2);
    assert(bar_counter == 3);

    return 0;
}();

} // namespace pigro::tests
