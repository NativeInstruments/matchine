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
//!  `bools` is a meta template container of compile time bool literals. Its used for meta-programming algorithms.
//! 
//!---------------------------------------------------------------------------------------------------------------------

#pragma once

namespace ni {

    namespace meta {

        template <bool...>
        struct bools {};

    }

}
