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
//!  fold_add<Integral, n1, n2,..., nN> is a template meta function that folds a add over all its arguments n1...nN of
//!  type Integral. I.e. it computes n1 + ... + nN. The name is chosen in alignment with the other fold operations.
//!
//!  C++ 17 makes this obsolete since it introduces fold expressions into the core language.
//!
//!---------------------------------------------------------------------------------------------------------------------

#pragma once

#include <type_traits>

namespace ni {

    namespace meta {

        template <typename Integral, Integral... ns>
        struct fold_add;

        template <typename Integral, Integral n>
        struct fold_add<Integral, n> : std::integral_constant<Integral, n> {};

        template <typename Integral, Integral n, Integral... ns>
        struct fold_add<Integral, n, ns...> : std::integral_constant
        <   Integral
        ,   n + fold_add<Integral, ns...>::value
        >
        {};

        template <typename Integral, Integral... ns>
        constexpr Integral fold_add_v = fold_add<Integral, ns...>::value;
    }

}
