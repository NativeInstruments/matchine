//
// MIT License
//
// Copyright © 2018
// Native Instruments
//
// For more detailed information, please read the LICENSE in the root directory.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - >8


//!---------------------------------------------------------------------------------------------------------------------
//!  \file
//!  \brief  A type trait for function signatures.
//!
//!   ni::signature has the following traits for function-esque types:
//!   * result_type         return type of this function
//!   * argument<N>::type   type of arguemt N 
//!   * num_args            number of arguments this function expects
//!
//!
//!   Example:
//!
//!       auto lambda = []( int i, string s ) -> float { return 3.14; }
//!       signature< decltype(lambda) >::result_type         // should be float;
//!       signature< decltype(lambda) >::argument<0>::type   // should be int;
//!       signature< decltype(lambda) >::argument<1>::type   // should be string;
//!       signature< decltype(lambda) >::num_args            // should be 2;
//!
//!   This file has originally been taken from here:  https://github.com/andre-bergner/funky
//!
//!---------------------------------------------------------------------------------------------------------------------

#pragma once

#include <boost/config.hpp>

#include <type_traits>
#include <tuple>


namespace ni
{
    // the generic signature falls back on the signature of the call operator if present
    template < class C >
    struct signature : signature< decltype( &std::decay_t<C>::operator() ) >  {};


    // pointer to member function (incl. const & volatile ) fall back on the plain function signatures
    template < class C , typename Result , typename... Args >
    struct signature< Result (C::*)(Args...) > : signature< Result ( Args... ) > {};

    template < class C , typename Result , typename... Args >
    struct signature< Result (C::*)(Args...) const > : signature< Result ( Args... ) > {};

    template < class C , typename Result , typename... Args >
    struct signature< Result (C::*)(Args...) volatile > : signature< Result ( Args... ) > {};

    template < class C , typename Result , typename... Args >
    struct signature< Result (C::*)(Args...) const volatile > : signature< Result ( Args... ) > {};


    // pointer and references to free function fall back on the plain function signatures
    template < typename Result , typename... Args >
    struct signature< Result (*)(Args...) > : signature< Result ( Args... ) > {};

    template < typename Result , typename... Args >
    struct signature< Result (&)(Args...) > : signature< Result ( Args... ) > {};


    // actual implementation just for pure function signature types
    template < typename Result , typename... Args >
    struct signature< Result ( Args... ) >
    {
        BOOST_STATIC_CONSTEXPR auto num_args = sizeof...(Args);

        typedef Result type( Args... );
        //using  type = Result( Args... );

        template < size_t n >
        using  argument = typename std::tuple_element< n, std::tuple<Args...> >;
        using  result_type = Result;

        static constexpr size_t number_of_arguments = sizeof...(Args);
    };

    template <typename Callable>
    using signature_t = typename signature<std::decay_t<Callable>>::type;

    template <typename Callable>
    using result_t = typename signature<std::decay_t<Callable>>::result_type;
}
