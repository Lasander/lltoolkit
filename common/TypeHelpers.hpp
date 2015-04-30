/**
 * TypeHelpers.hpp
 *
 *  Created on: Apr 29, 2015
 *      Author: lasse
 */

#ifndef COMMON_TYPEHELPERS_HPP_
#define COMMON_TYPEHELPERS_HPP_

#include <tuple>
#include <type_traits>

namespace Common {

template <typename T, typename Tuple>
struct has_type;
template <typename T>
struct has_type<T, std::tuple<>> : std::false_type {};
template <typename T, typename U, typename... Ts>
struct has_type<T, std::tuple<U, Ts...>> : has_type<T, std::tuple<Ts...>> {};
template <typename T, typename... Ts>
struct has_type<T, std::tuple<T, Ts...>> : std::true_type {};

/**
 * Check whether a tuple contains the given type.
 *
 * @tparam T Type to search
 * @tparam Tuple Tuple to check
 * @return tuple_contains_type<T, Tuple>::value is true if the type is found in the tuple
 *
 * E.g.:
 * static_assert(tuple_contains_type<int, std::tuple<int, float>>::value, "Tuple didn't contain int");
 */
template <typename T, typename Tuple>
using tuple_contains_type = typename has_type<T, Tuple>::type;



template <typename Tuple>
struct has_duplicate;
template <>
struct has_duplicate<std::tuple<>> : std::false_type {};
template <typename T, typename... Ts>
struct has_duplicate<std::tuple<T, Ts...>> : has_duplicate<std::tuple<Ts...>> { static_assert(!tuple_contains_type<T, std::tuple<Ts...>>::value, "Tuple contains a duplicate"); };

/**
 * Check whether a tuple contains duplicate types.
 *
 * @tparam Tuple Tuple to check
 * @return tuple_contains_duplicates<Tuple>::value is true if the tuple contains duplicate types
 *
 * E.g.:
 * static_assert(tuple_contains_duplicates<std::tuple<int, float>>::value, "Tuple didn't contain duplicates");
 */
template <typename Tuple>
using tuple_contains_duplicates = typename has_duplicate<Tuple>::type;

} // namespace Common

#endif /* COMMON_TYPEHELPERS_HPP_ */
