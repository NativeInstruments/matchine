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
//!  fold_or<b1,b2,...,bN> is a template meta function that folds a logical or over all its bool arguments b1...bN.
//!
//!  I.e. fold<b1,...,bN>::value is true if at least one value of b1,...,bN is true.
//!
//!  C++ 17 makes this obsolete since it introduces fold expressions into the core language.
//!
//!---------------------------------------------------------------------------------------------------------------------

#pragma once

#include <ni/meta/fold_and.h>

namespace ni {

    namespace meta {

        template <bool... bs>
        struct fold_or : std::integral_constant
        <   bool
        ,   not fold_and< (not bs)... >::value
        >
        {};

        template <bool... bs>
        constexpr bool fold_or_v = fold_or<bs...>::value;
    }

}
