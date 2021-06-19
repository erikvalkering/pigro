#include "../src/pigro/overload.h"

#include <cassert>
#include <iostream>

using namespace std;

namespace pigro::tests {

auto test_overload = [] {
    cout << "test_overload" << endl;

    const auto f = overload{
        [](int) { return "int"s; },
        [](double) { return "double"s; },
        [](auto) { return "auto"s; },
    };

    assert(f(0) == "int");
    assert(f(0.0) == "double");
    assert(f(true) == "auto");

    return 0;
}();

auto test_ebo_msvc_workaround = [] {
    cout << "test_ebo_msvc_workaround" << endl;

    auto r1 = overload{
        [] {},
        [] {},
    };
    auto r2 = overload{
        [] {},
        [x = 1] { return x; },
    };
    auto r3 = overload{
        [x = 'a'] { return x; },
        [] {},
    };
    auto r4 = overload{
        [] {},
        [x = 1] { return x; },
        [] {},
        [x = 1] { return x; },
    };
    auto r5 = overload{
        [x = 1] { return x; },
        [] {},
        [x = 1] { return x; },
        [] {},
    };

    static_assert(is_empty_v<decltype(r1)>);
    static_assert(sizeof(r1) == sizeof(char));
    static_assert(!is_empty_v<decltype(r2)>);
    static_assert(sizeof(r2) == sizeof(int));
    static_assert(!is_empty_v<decltype(r3)>);
    static_assert(sizeof(r3) == sizeof(char));
    static_assert(!is_empty_v<decltype(r4)>);
    static_assert(sizeof(r4) == 2 * sizeof(int));
    static_assert(!is_empty_v<decltype(r5)>);
    static_assert(sizeof(r5) == 2 * sizeof(int));

    return 0;
}();

} // namespace pigro::tests
