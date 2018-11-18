//
// MIT License
//
// Copyright © 2018
// Native Instruments
//
// For more detailed information, please read the LICENSE in the root directory.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - >8

#include <gtest/gtest.h>

#include <ni/meta/fold_and.h>
#include <ni/meta/fold_or.h>
#include <ni/meta/fold_add.h>
#include <ni/meta/scan_add.h>


TEST( ni_meta_fold , fold_and_scenario )
{
    using ni::meta::fold_and_v;
    EXPECT_TRUE((  fold_and_v<> ));
    EXPECT_TRUE((  fold_and_v<true> ));
    EXPECT_FALSE(( fold_and_v<false> ));
    EXPECT_FALSE(( fold_and_v<true,false> ));
    EXPECT_FALSE(( fold_and_v<true,true,true,false,true> ));
}

TEST( ni_meta_fold , fold_or_scenario )
{
    using ni::meta::fold_or_v;
    EXPECT_FALSE(( fold_or_v<> ));
    EXPECT_TRUE((  fold_or_v<true> ));
    EXPECT_FALSE(( fold_or_v<false> ));
    EXPECT_TRUE((  fold_or_v<true,false> ));
    EXPECT_TRUE((  fold_or_v<true,true> ));
    EXPECT_FALSE(( fold_or_v<false,false> ));
}

TEST( ni_meta_fold , fold_add_scenario )
{
    using ni::meta::fold_add_v;
    EXPECT_EQ( 1337, (fold_add_v<int, 1337>) );
    EXPECT_EQ( 6, (fold_add_v<int, 1,2,3>) );
    EXPECT_EQ( 0, (fold_add_v<int, -1, 1, -2, 2>) );
}

TEST( ni_meta_scan , scan_add_scenario )
{
    using ni::meta::scan_add_t;
    using namespace std;

    EXPECT_TRUE(( is_same< integer_sequence<int, 1>,            scan_add_t<int, 1> >::value ));
    EXPECT_TRUE(( is_same< integer_sequence<int, 1, 3>,         scan_add_t<int, 1, 2> >::value ));
    EXPECT_TRUE(( is_same< integer_sequence<int, 1, 3, 6, 10>,  scan_add_t<int, 1, 2, 3, 4> >::value ));

    EXPECT_TRUE(( is_same< integer_sequence<int, 0, 1>, scan_add_t<int, 0, 1> >::value ));
    EXPECT_TRUE(( is_same< integer_sequence<int, 0, 1, 2>, scan_add_t<int, 0, 1, 1> >::value ));
    EXPECT_TRUE(( is_same< integer_sequence<int, 0, 1, 2, 3>, scan_add_t<int, 0, 1, 1, 1> >::value ));
}
