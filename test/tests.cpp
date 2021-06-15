#include <cassert>
#include <iostream>

using namespace std;

namespace pigro {

constexpr auto lazy = [](auto f) {
    return [] { return 0; };
};

} // namespace pigro

namespace pigro::tests {

auto test_cached() {
    cout << "test_cached" << endl;

    auto counter = 0;
    auto foo = lazy([&] {
        ++counter;
        return 0;
    });

    assert(counter == 0);
    assert(foo() == 0);
    assert(counter == 1);

    assert(foo() == 0);
    assert(counter == 1);
}

auto test_pigro() {
    test_cached();
}

} // namespace pigro::tests

int main() {
    using namespace pigro::tests;

    test_pigro();
}
