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
//!  fold_and<b1,b2,...,bN> is a template meta function that folds a logical and over all its bool arguments b1...bN.
//!
//!  I.e. fold<b1,...,bN>::value is only true iff b1 == ... == bN == true.
//!
//!  C++ 17 makes this obsolete since it introduces fold expressions into the core language.
//!
//!---------------------------------------------------------------------------------------------------------------------

#pragma once

#include <ni/meta/bools.h>

namespace ni {

    namespace meta {

        namespace detail {

            template <bool b>
            struct as_true { static constexpr bool value = true; };

        }

        template <bool... bs>
        struct fold_and : std::integral_constant
        <   bool
        ,   std::is_same
            <  bools< detail::as_true<bs>::value... >
            ,  bools< bs... >
            >::value
        >
        {};

        template <bool... bs>
        constexpr bool fold_and_v = fold_and<bs...>::value;
    }

}
