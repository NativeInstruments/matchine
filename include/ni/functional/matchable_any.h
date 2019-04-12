//
// MIT License
//
// Copyright © 2018
// Native Instruments
//
// For more detailed information, please read the LICENSE in the root directory.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - >8
//
// This is a customization point to ni::match for std::any and makes instances
// of std::any matchable.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - >8

#include <any>


namespace std
{
    template <typename Target>
    Target const* dyn_cast(any const* a)
    {
        return any_cast<Target const>(a);
    }

    template <typename Target>
    Target* dyn_cast(any* a)
    {
        return any_cast<Target>(a);
    }
}
