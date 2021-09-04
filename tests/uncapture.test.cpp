#include "../src/pigro/overload.h"
#include "../src/pigro/uncapture.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

#include <type_traits>

using namespace boost::ut;

namespace pigro::tests {

struct Empty {
    auto operator<=>(const Empty &) const = default;
};

suite uncapture_tests = [] {
    auto empty = Empty{};
};

} // namespace pigro::tests
