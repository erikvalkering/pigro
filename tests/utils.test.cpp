#include "../src/pigro/utils.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

using namespace boost::ut;

namespace pigro::tests {

template<typename...>
struct Foo;

template<typename...>
struct Bar;

suite utils_tests = [] {
    "rebind_container_t"_test = [] {
        expect(type<rebind_container_t<Foo<>, Bar>> == type<Bar<>>);
        expect(type<rebind_container_t<Foo<int>, Bar>> == type<Bar<int>>);
        expect(type<rebind_container_t<Foo<int, float>, Bar>> == type<Bar<int, float>>);
    };
};

} // namespace pigro::tests
