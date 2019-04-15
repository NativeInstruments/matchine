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

#include <gtest/gtest.h>

#include <memory>
#include <vector>

//----------------------------------------------------------------------------------------------------------------------

TEST( ni_match, matcher_on_polymorphic_types )
{
    struct base { virtual ~base(){} };
    struct derived1 : base { const int value1 = 1337; };
    struct derived2 : base { const int value2 = 7357; };

    derived1 d1;
    derived2 d2;

    auto f = ni::matcher
    (   [](derived1& x) { return x.value1 + 1; }
    ,   [](derived2& x) { return x.value2 + 2; }
    );

    EXPECT_EQ( 1337 + 1, f(static_cast<base&>(d1)) );
    EXPECT_EQ( 7357 + 2, f(static_cast<base&>(d2)) );
}

//----------------------------------------------------------------------------------------------------------------------

TEST( ni_match, match_polymorphic_types )
{
    struct base { virtual ~base(){} };
    struct derived1 : base { const int value1 = 1337; };
    struct derived2 : base { const int value2 = 7357; };

    const auto objects = std::vector<std::shared_ptr<base>>
    {   std::make_shared<derived1>()
    ,   std::make_shared<derived2>()
    };

    const std::vector<int>  expected {1337 + 1, 7357 + 2};
    std::vector<int>        results;

    for (auto& obj : objects)
        ni::match(*obj)
        (   [&](derived1& x) { results.push_back(x.value1 + 1); }
        ,   [&](derived2& x) { results.push_back(x.value2 + 2); }
        );

    EXPECT_EQ( expected, results );
}

//----------------------------------------------------------------------------------------------------------------------

namespace ni_match_test_detail
{
    struct base
    {
        base(int t) : dyn_tag(t) {}

        template <typename Target>
        const Target* cast_to() const
        {
            return dyn_tag == Target::tag ? static_cast<Target const*>(this) : nullptr;
        }

        template <typename Target>
        Target* cast_to()
        {
            return dyn_tag == Target::tag ? static_cast<Target*>(this) : nullptr;
        }

        const int dyn_tag;
    };

    template <typename TargetType>
    TargetType* dyn_cast(base* b) { return b->cast_to<TargetType>(); }

    template <typename TargetType>
    TargetType const* dyn_cast(base const* b) { return b->cast_to<TargetType>(); }



    struct derived1 : base
    {
        static const int tag = 1;
        derived1() : base(tag) {}
        const int value = 1337;
    };

    struct derived2 : base
    {
        static const int tag = 2;
        derived2() : base(tag) {}
        const int value = 7357;
    };
}

TEST( ni_match, match_custom_polymorhic_types )
{
    using namespace ni_match_test_detail;

    const auto objects = std::vector<std::shared_ptr<base>>
    {   std::make_shared<derived1>()
    ,   std::make_shared<derived2>()
    };

    const std::vector<int>  expected {1337, 7357};
    std::vector<int>        results;

    for (auto& obj : objects)
        ni::match(*obj)
        (   [&](derived1& x) { results.push_back(x.value); }
        ,   [&](derived2& x) { results.push_back(x.value); }
        );

    EXPECT_EQ( expected, results );
}

//----------------------------------------------------------------------------------------------------------------------

TEST( ni_match, match_with_otherwise_value )
{
    struct base { virtual ~base(){} };
    struct derived1 : base { const int value1 = 1337; };
    struct derived2 : base { const int value2 = 7357; };
    struct derived3 : base { };

    derived1 d1;
    derived2 d2;
    derived3 d3;

    auto f = ni::matcher
    (   [](derived1& x) { return x.value1 + 1; }
    ,   [](derived2& x) { return x.value2 + 2; }
    ,   ni::otherwise(-1)
    //,   []{ return -1; }      // this also works
    );

    static_assert(
        std::is_same<decltype(f(d1)), int>::value,
        "When using 'otherwise' the result type must be int."
    );

    EXPECT_EQ( 1337 + 1, f(static_cast<base&>(d1)) );
    EXPECT_EQ( 7357 + 2, f(static_cast<base&>(d2)) );
    EXPECT_EQ( -1, f(static_cast<base&>(d3)) );
}

//----------------------------------------------------------------------------------------------------------------------

TEST( ni_match, otherwise_value_can_be_at_any_position )
{
    struct base { virtual ~base(){} };
    struct derived1 : base { const int value1 = 1337; };
    struct derived2 : base { const int value2 = 7357; };
    struct derived3 : base { };

    derived1 d1;
    derived2 d2;
    derived3 d3;

    auto f = ni::matcher
    (   ni::otherwise(-1)
    ,   [](derived1& x) { return x.value1 + 1; }
    ,   [](derived2& x) { return x.value2 + 2; }
    );

    EXPECT_EQ( 1337 + 1, f(static_cast<base&>(d1)) );
    EXPECT_EQ( 7357 + 2, f(static_cast<base&>(d2)) );
    EXPECT_EQ( -1, f(static_cast<base&>(d3)) );
}

//----------------------------------------------------------------------------------------------------------------------

namespace ni_match_test_detail
{
    struct AnyNumber
    {
        enum Tag { VOID, INT, FLOAT };
        union { int i_; float f_; };
        Tag tag = VOID;

        AnyNumber() = default;
        AnyNumber(int i) : i_{i} , tag{INT} {}
        AnyNumber(float f) : f_{f} , tag{FLOAT} {}

        operator int*()               { return tag == INT ? &i_ : nullptr; }
        operator float*()             { return tag == FLOAT ? &f_ : nullptr; }
        operator int const*() const   { return tag == INT ? &i_ : nullptr; }
        operator float const*() const { return tag == FLOAT ? &f_ : nullptr; }
    };

    template <typename TargetType>
    TargetType* dyn_cast(AnyNumber* n) { return (TargetType*)(*n); }

    template <typename TargetType>
    TargetType const* dyn_cast(AnyNumber const* n) { return (TargetType const*)(*n); }
}

TEST( ni_match, match_on_rvalues )
{
    using ni_match_test_detail::AnyNumber;

    auto get_int = []{ return AnyNumber{1337}; };
    auto get_float = []{ return AnyNumber{3.14f}; };

    EXPECT_TRUE(ni::match(get_int())([](int& i){ EXPECT_EQ(1337, i); } ));
    EXPECT_TRUE(ni::match(get_float())([](float& f){ EXPECT_EQ(3.14f, f); } ));
    EXPECT_TRUE(ni::match(get_int())([](int const& i){ EXPECT_EQ(1337, i); } ));
    EXPECT_TRUE(ni::match(get_float())([](float const& f){ EXPECT_EQ(3.14f, f); } ));
}
