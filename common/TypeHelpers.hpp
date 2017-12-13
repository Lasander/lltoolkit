#pragma once

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace Common {

template <typename T, typename Tuple>
struct has_type;
template <typename T>
struct has_type<T, std::tuple<>> : std::false_type
{
};
template <typename T, typename U, typename... Ts>
struct has_type<T, std::tuple<U, Ts...>> : has_type<T, std::tuple<Ts...>>
{
};
template <typename T, typename... Ts>
struct has_type<T, std::tuple<T, Ts...>> : std::true_type
{
};

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
struct has_duplicate<std::tuple<>> : std::false_type
{
};
template <typename T, typename... Ts>
struct has_duplicate<std::tuple<T, Ts...>> : has_duplicate<std::tuple<Ts...>>
{
    static_assert(!tuple_contains_type<T, std::tuple<Ts...>>::value, "Tuple contains a duplicate");
};

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

template <class T, std::size_t N, class... Args>
struct get_number_of_element_from_tuple_by_type_impl
{
    static constexpr auto value = N;
};

template <class T, std::size_t N, class... Args>
struct get_number_of_element_from_tuple_by_type_impl<T, N, T, Args...>
{
    static constexpr auto value = N;
};

template <class T, std::size_t N, class U, class... Args>
struct get_number_of_element_from_tuple_by_type_impl<T, N, U, Args...>
{
    static constexpr auto value = get_number_of_element_from_tuple_by_type_impl<T, N + 1, Args...>::value;
};

template <class T, class... Args>
T get_element_by_type(const std::tuple<Args...>& t)
{
    return std::get<get_number_of_element_from_tuple_by_type_impl<T, 0, Args...>::value>(t);
}

template <typename T, typename... Args>
std::unique_ptr<T> make_unique_helper(std::false_type, Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T, typename... Args>
std::unique_ptr<T> make_unique_helper(std::true_type, Args&&... args)
{
    static_assert(std::extent<T>::value == 0, "make_unique<T[N]>() is forbidden, please use make_unique<T[]>().");

    typedef typename std::remove_extent<T>::type U;
    return std::unique_ptr<T>(new U[sizeof...(Args)]{std::forward<Args>(args)...});
}

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return make_unique_helper<T>(std::is_array<T>(), std::forward<Args>(args)...);
}

template <typename... Ts>
struct make_void
{
    typedef void type;
};
template <typename... Ts>
using void_t = typename make_void<Ts...>::type;

} // namespace Common
