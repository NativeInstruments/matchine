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
//!  \brief  meta::dispatch<n> allows to construct tag-dispatch hierachies
//!
//!  Example:
//!
//!  \code
//!     template< typename X, typename Y >
//!     auto set_value( dispatch<2>, X& x, Y const &y ) -> decltype( x.set(y), void ) { x.set(y) }
//!
//!     template< typename X, typename Y >
//!     auto set_value( dispatch<1>, X& x, Y const &y ) -> decltype( x.Set(y), void ) { x.Set(y); }
//!
//!     template< typename X, typename Y >
//!     auto set_value( dispatch<0>, X& x, Y const &y ) -> decltype( x.assign(y), void ) { x.assign(y); }
//!  \endcode
//!
//!---------------------------------------------------------------------------------------------------------------------

#pragma once

namespace ni {

    namespace meta {

        template< size_t level = 0 >  struct dispatch : dispatch< level-1 > {};
        template<>                    struct dispatch<0> {};

    }

}
