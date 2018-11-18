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
//!
//!  scan_add<Integral, init, n1, n2,..., nN> is a template meta function that performs a scan with operation + over
//!  all its arguments n1...nN of type Integral. I.e. it return <init, init+n1, init+n1+n2, init+n1+...+nN.
//!
//!---------------------------------------------------------------------------------------------------------------------

#pragma once

#include <type_traits>

namespace ni {

    namespace meta {

        template <typename Integral, Integral... Ns>
        struct scan_add;

        template <typename Integral, Integral... Ns>
        using scan_add_t = typename scan_add<Integral, Ns...>::type;


        //--------------------------------------------------------------------------------------------------------------
        // impl
        //--------------------------------------------------------------------------------------------------------------

        template <typename Integral, typename Result, Integral... Ns>
        struct scan_add_impl;

        template <typename Integral, Integral N, Integral... Ms>
        struct scan_add_impl
        <   Integral
        ,   std::integer_sequence<Integral, Ms...>
        ,   N
        >
        {
            using type = std::integer_sequence<Integral, Ms..., N>;
        };

        template <typename Integral, Integral N1, Integral N2, Integral... Ns, Integral... Ms>
        struct scan_add_impl
        <   Integral
        ,   std::integer_sequence<Integral, Ms...>
        ,   N1, N2, Ns...
        >
        {
            using type = typename scan_add_impl<Integral, std::integer_sequence<Integral, Ms..., N1>, N1+N2, Ns...>::type;
        };

        template <typename Integral, Integral... Ns>
        struct scan_add
        {
            using type = typename scan_add_impl< Integral, std::integer_sequence<Integral>, Ns...>::type;
        };

    }

}
