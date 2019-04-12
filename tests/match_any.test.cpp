//
// MIT License
//
// Copyright © 2018
// Native Instruments
//
// For more detailed information, please read the LICENSE in the root directory.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - >8

#include <ni/functional/match.h>
#include <ni/functional/matchable_any.h>

#include <gtest/gtest.h>


TEST( ni_match, match_any )
{
    std::any any;

    auto m = ni::matcher
    (   [](int x)    { return int(x) + 1; }
    ,   [](double x) { return int(x) + 2; }
    ,   []           { return -42; }
    );

    EXPECT_EQ( -42, m(any) );

    any = 1336;
    EXPECT_EQ( 1337, m(any) );

    any = 7355.;
    EXPECT_EQ( 7357, m(any) );

    auto const& const_any = any;
    EXPECT_EQ( 7357, m(const_any) );
}

