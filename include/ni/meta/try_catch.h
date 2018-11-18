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
//!  Meta try-catch is a convenience version of the more generic meta::dispatch for two levels only.
//!
//!  Example:
//!
//!  ```
//!  template< typename X, typename Y >
//!  auto is_assignable( try_t, X& x, Y const &y ) -> decltype( x=y, true ) { return true; }
//!
//!  template< typename X, typename Y >
//!  auto is_assignable( catch_t, X& x, Y const &y ) -> decltype( false )  { return false; }
//!  ```
//!
//!---------------------------------------------------------------------------------------------------------------------

#pragma once

#include <ni/meta/dispatch.h>


namespace ni {

    namespace meta {

        using try_t   = dispatch<1>;
        using catch_t = dispatch<0>;

    }

}
