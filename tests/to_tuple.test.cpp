#include "../src/pigro/compressed_tuple.h"
#include "../src/pigro/to_tuple.h"

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

#include <tuple>

using namespace boost::ut;

namespace pigro::tests {

suite to_tuple_tests = [] {
    "empty_parameters"_test = [] {
        expect(type<decltype(to_tuple(std::declval<compressed_tuple<> &>()))> == type<std::tuple<>>);
        expect(type<decltype(to_tuple(std::declval<compressed_tuple<> &&>()))> == type<std::tuple<>>);
        expect(type<decltype(to_tuple(std::declval<const compressed_tuple<> &>()))> == type<std::tuple<>>);
        expect(type<decltype(to_tuple(std::declval<const compressed_tuple<> &&>()))> == type<std::tuple<>>);
    };

    "single parameter (changing)"_test = [] {
        expect(type<decltype(to_tuple(std::declval<compressed_tuple<int> &>()))> == type<std::tuple<int &>>);
        expect(type<decltype(to_tuple(std::declval<compressed_tuple<int> &&>()))> == type<std::tuple<int &&>>);
        expect(type<decltype(to_tuple(std::declval<compressed_tuple<int &> &>()))> == type<std::tuple<int &>>);
        expect(type<decltype(to_tuple(std::declval<compressed_tuple<int &> &&>()))> == type<std::tuple<int &&>>);
        expect(type<decltype(to_tuple(std::declval<compressed_tuple<int &&> &>()))> == type<std::tuple<int &>>);
        expect(type<decltype(to_tuple(std::declval<compressed_tuple<int &&> &&>()))> == type<std::tuple<int &&>>);
        expect(type<decltype(to_tuple(std::declval<compressed_tuple<const int &> &>()))> == type<std::tuple<const int &>>);
        expect(type<decltype(to_tuple(std::declval<compressed_tuple<const int &> &&>()))> == type<std::tuple<const int &&>>);
        expect(type<decltype(to_tuple(std::declval<compressed_tuple<const int &&> &>()))> == type<std::tuple<const int &>>);
        expect(type<decltype(to_tuple(std::declval<compressed_tuple<const int &&> &&>()))> == type<std::tuple<const int &&>>);
        expect(type<decltype(to_tuple(std::declval<const compressed_tuple<int> &>()))> == type<std::tuple<const int &>>);
        expect(type<decltype(to_tuple(std::declval<const compressed_tuple<int> &&>()))> == type<std::tuple<const int &&>>);
        expect(type<decltype(to_tuple(std::declval<const compressed_tuple<int &> &>()))> == type<std::tuple<const int &>>);
        expect(type<decltype(to_tuple(std::declval<const compressed_tuple<int &> &&>()))> == type<std::tuple<const int &&>>);
        expect(type<decltype(to_tuple(std::declval<const compressed_tuple<int &&> &>()))> == type<std::tuple<const int &>>);
        expect(type<decltype(to_tuple(std::declval<const compressed_tuple<int &&> &&>()))> == type<std::tuple<const int &&>>);
        expect(type<decltype(to_tuple(std::declval<const compressed_tuple<const int &> &>()))> == type<std::tuple<const int &>>);
        expect(type<decltype(to_tuple(std::declval<const compressed_tuple<const int &> &&>()))> == type<std::tuple<const int &&>>);
        expect(type<decltype(to_tuple(std::declval<const compressed_tuple<const int &&> &>()))> == type<std::tuple<const int &>>);
        expect(type<decltype(to_tuple(std::declval<const compressed_tuple<const int &&> &&>()))> == type<std::tuple<const int &&>>);
    };
};

} // namespace pigro::tests
