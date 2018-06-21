#ifndef RESULT_TRAITS_HPP_INCLUDED
#define RESULT_TRAITS_HPP_INCLUDED

#include <type_traits>

namespace result { 

    template<typename T, typename E>
    struct Result;

namespace traits {


    template<typename T>
    struct is_result : std::false_type
    { };

    template<typename T, typename E>
    struct is_result<Result<T, E>> : std::true_type
    { };

    template<typename T>
    struct result_traits;

    template<typename T, typename E>
    struct result_traits<Result<T, E>> {
        using value_type = T;
        using error_type = E;
        using value_is_empty = std::is_same<void, T>;
        using value_is_result = is_result<T>;
        using error_is_result = is_result<E>;
    };

    template<typename T>
    struct inner_result_value_traits;

    template<typename T, typename E>
    struct inner_result_value_traits<Result<T, E>> {

        template<typename Val, typename Err>
        struct inner_impl {
            using value_type = Val;
            using error_type = Err;
        };

        template<typename Err, typename Val_, typename Err_>
        struct inner_impl<Result<Val_, Err_>, Err> {
            using value_type = 
                typename inner_impl<Val_, Err_>::value_type;
            using error_type = 
                typename inner_impl<Val_, Err_>::error_type;
        };

        using value_type = 
            typename inner_impl<T, E>::value_type;

        using error_type = 
            typename inner_impl<T, E>::error_type;
    };

    template<typename T>
    struct inner_result_error_traits;

    template<typename T, typename E>
    struct inner_result_error_traits<Result<T, E>> {

        template<typename Val, typename Err>
        struct inner_impl {
            using value_type = Val;
            using error_type = Err;
        };

        template<typename Val, typename Val_, typename Err_>
        struct inner_impl<Val, Result<Val_, Err_>> {
            using value_type = 
                typename inner_impl<Val_, Err_>::value_type;
            using error_type = 
                typename inner_impl<Val_, Err_>::error_type;
        };

        using value_type = 
            typename inner_impl<T, E>::value_type;

        using error_type = 
            typename inner_impl<T, E>::error_type;
    };
}}

#endif //RESULT_TRAITS_HPP_INCLUDED
