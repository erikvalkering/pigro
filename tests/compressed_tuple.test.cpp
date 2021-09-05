#include "../src/pigro/compressed_tuple.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

#include <tuple>
#include <type_traits>

using namespace boost::ut;

namespace pigro::tests {

struct Empty {
    Empty() = default;
    auto operator<=>(const Empty &) const = default;
};

suite compressed_tuple_tests = [] {
    auto empty = Empty{};

    "basic"_test = [=] {
        auto t1 = compressed_tuple{};
        expect(constant<sizeof(t1) == sizeof(empty)>);
        expect(constant<std::is_empty_v<decltype(t1)>>);

        auto x = int{ 1 };
        auto t2 = compressed_tuple{ x };

        expect(std::get<0>(t2) == x);
        expect(constant<sizeof(t2) == sizeof(int)>);
        expect(constant<!std::is_empty_v<decltype(t2)>>);

        auto y = double{ 2.0 };
        auto t3 = compressed_tuple{ x, y };

        expect(std::get<0>(t3) == x);
        expect(std::get<1>(t3) == y);
        expect(constant<sizeof(t3) == sizeof(double) + sizeof(double)>);
        expect(constant<!std::is_empty_v<decltype(t3)>>);

        auto t4 = compressed_tuple{ empty };

        expect(std::get<0>(t4) == empty);
        expect(constant<sizeof(t4) == sizeof(empty)>);
        expect(constant<std::is_empty_v<decltype(t4)>>);

        auto t5 = compressed_tuple{ empty, y };

        expect(std::get<0>(t5) == empty);
        expect(std::get<1>(t5) == y);
        expect(constant<sizeof(t5) == 0 + sizeof(double)>);
        expect(constant<!std::is_empty_v<decltype(t5)>>);

        auto t6 = compressed_tuple{ y, empty };

        expect(std::get<0>(t6) == y);
        expect(std::get<1>(t6) == empty);
        expect(constant<sizeof(t6) == sizeof(double) + 0>);
        expect(constant<!std::is_empty_v<decltype(t6)>>);
    };

    "CTAD"_test = [] {
        const auto t1 = compressed_tuple{};
        expect(type<>(t1) == type<compressed_tuple<>>);

        const auto t2 = compressed_tuple{ 1 };
        expect(type<>(t2) == type<compressed_tuple<int>>);

        auto x = 0;
        const auto t3 = compressed_tuple{ x };
        expect(type<>(t3) == type<compressed_tuple<int &>>);

        const auto y = 0;
        const auto t4 = compressed_tuple{ y };
        expect(type<>(t4) == type<compressed_tuple<const int &>>);
    };
};

} // namespace pigro::tests
