#include "common/TypeHelpers.hpp"
#include "gtest/gtest.h"

namespace Common {

TEST(TypeHelpersTest, tupleHasType)
{
    std::tuple<float, int> myTuple(1.0, 2);
    static_assert(tuple_contains_type<float, decltype(myTuple)>::value, "Tuple does not contain expected type");
    static_assert(tuple_contains_type<int, decltype(myTuple)>::value, "Tuple does not contain expected type");
    static_assert(!tuple_contains_type<double, decltype(myTuple)>::value, "Tuple contains unexpected type");
    static_assert(!tuple_contains_type<unsigned int, decltype(myTuple)>::value, "Tuple contains unexpected type");
}

TEST(TypeHelpersTest, tupleUniqueness)
{
    std::tuple<float, int> myTuple(1.0, 2);
    static_assert(!has_duplicate<decltype(myTuple)>::value, "Tuple is not unique");

    std::tuple<int, float, int> myTuple2(1, 1.0, 2);
    // static_assert(has_duplicate<decltype(myTuple)>::value, "Tuple is unique"); // fails to compile
}

} // namespace Common
