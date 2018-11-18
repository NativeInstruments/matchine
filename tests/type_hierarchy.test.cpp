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

#include <ni/type_hierarchy.h>
#include <ni/functional/match.h>
#include <ni/meta/try_catch.h>

#include <vector>

//----------------------------------------------------------------------------------------------------------------------

struct TypeHierarchyTest : public ::testing::Test
{
    struct TestHierarchyBase {};

    using Root = ni::type_hierarchy::from_base<TestHierarchyBase>;

    template <typename TargetType>
    static bool convertible_to(Root const& r) { return ni::convertible_to<TargetType>(r); }

    template <typename TargetType>
    static bool convertible_to(Root const* r) { return ni::convertible_to<TargetType>(*r); }

    struct Type_1 : ni::sub_type<Type_1, Root> {};
    struct Type_1_1 : ni::sub_type<Type_1_1, Type_1> {};
    struct Type_1_1_1 : ni::sub_type<Type_1_1_1, Type_1_1> {};
    struct Type_1_1_2 : ni::sub_type<Type_1_1_2, Type_1_1> {};
    struct Type_1_2 : ni::sub_type<Type_1_2, Type_1> {};

    struct Type_2 : ni::sub_type<Type_2, Root> {};
    struct Type_2_1 : ni::sub_type<Type_2_1, Type_2> {};
    struct Type_2_2 : ni::sub_type<Type_2_2, Type_2> {};


    Type_1      x_1, x_1a;
    Type_1_1    x_1_1;
    Type_1_2    x_1_2;
    Type_1_1_1  x_1_1_1;
    Type_1_1_2  x_1_1_2;

    Type_2      x_2;
    Type_2_1    x_2_1;
    Type_2_2    x_2_2;
};

//----------------------------------------------------------------------------------------------------------------------

TEST_F(TypeHierarchyTest, type_is_not_convertible_to_other_types_on_same_level)
{
    Root* r = &x_1;
    EXPECT_TRUE( convertible_to<Type_1>(r) );
    EXPECT_FALSE( convertible_to<Type_2>(r) );

    r = &x_2;
    EXPECT_FALSE( convertible_to<Type_1>(r) );
    EXPECT_TRUE( convertible_to<Type_2>(r) );

    r = &x_1_1_1;
    EXPECT_TRUE( convertible_to<Type_1_1_1>(r) );
    EXPECT_FALSE( convertible_to<Type_1_1_2>(r) );

    r = &x_2_1;
    EXPECT_TRUE( convertible_to<Type_2_1>(r) );
    EXPECT_FALSE( convertible_to<Type_2_2>(r) );
}

//----------------------------------------------------------------------------------------------------------------------

TEST_F(TypeHierarchyTest, type_on_level_2_is_convertible_to_supertype_on_level_1)
{
    Root* r = &x_1_1;
    EXPECT_TRUE( convertible_to<Type_1>(r) );

    r = &x_1_2;
    EXPECT_TRUE( convertible_to<Type_1>(r) );

    r = &x_2_1;
    EXPECT_TRUE( convertible_to<Type_2>(r) );
}

//----------------------------------------------------------------------------------------------------------------------

TEST_F(TypeHierarchyTest, type_on_level_2_is_not_convertible_to_any_other_type_on_level_1_but_supertype)
{
    Root* r = &x_1_1;
    EXPECT_FALSE( convertible_to<Type_2>(r) );

    r = &x_1_2;
    EXPECT_FALSE( convertible_to<Type_2>(r) );

    r = &x_2_1;
    EXPECT_FALSE( convertible_to<Type_1>(r) );
}

//----------------------------------------------------------------------------------------------------------------------

TEST_F(TypeHierarchyTest, type_on_level_3_is_convertible_to_all_super_types)
{
    Root& r = x_1_1_1;
    EXPECT_TRUE( convertible_to<TestHierarchyBase>(r) );
    EXPECT_TRUE( convertible_to<Type_1>(r) );
    EXPECT_TRUE( convertible_to<Type_1_1>(r) );
}

//----------------------------------------------------------------------------------------------------------------------

TEST_F(TypeHierarchyTest, reference_of_type_on_intermediate_levels_should_be_convertible_to_other_supported_levels )
{
    Type_1& r = x_1_1_1;
    EXPECT_TRUE( convertible_to<TestHierarchyBase>(r) );
    EXPECT_TRUE( convertible_to<Type_1>(r) );
    EXPECT_TRUE( convertible_to<Type_1_1>(r) );
    EXPECT_TRUE( convertible_to<Type_1_1_1>(r) );
}

//----------------------------------------------------------------------------------------------------------------------

TEST_F(TypeHierarchyTest, convertibility_from_const_to_nonconst)
{
    Root const* r = &x_1_1;
    EXPECT_TRUE( convertible_to<Type_1>(r) );
    EXPECT_FALSE( convertible_to<Type_2>(r) );
}

//----------------------------------------------------------------------------------------------------------------------

TEST_F(TypeHierarchyTest, convertibility_from_nonconst_to_const)
{
    Root* r = &x_1_1;
    EXPECT_TRUE( convertible_to<Type_1 const>(r) );
    EXPECT_FALSE( convertible_to<Type_2 const>(r) );
}

//----------------------------------------------------------------------------------------------------------------------

TEST_F(TypeHierarchyTest, convertibility_from_const_to_const)
{
    Root const* r = &x_1_1;
    EXPECT_TRUE( convertible_to<Type_1 const>(r) );
    EXPECT_FALSE( convertible_to<Type_2 const>(r) );
}




//----------------------------------------------------------------------------------------------------------------------
// testing with ni::match
//----------------------------------------------------------------------------------------------------------------------

TEST_F(TypeHierarchyTest, match_flat)
{
    std::vector<Root*>       ptrs             = {&x_1, &x_1_1, &x_1_2, &x_1_1_1, &x_1_1_2, &x_2, &x_2_1, &x_2_2};
    std::vector<std::string> expected_matches = {  "1",  "1_1",  "1_2",  "1_1_1",  "1_1_2",  "2",  "2_1",  "2_2"};
    std::vector<std::string> matches;

    auto rec = [&matches](std::string s) { matches.push_back(s); };

    for (Root* p : ptrs)
    {
        ni::match(*p)
        (   [&](Type_1_1_2)    { rec("1_1_2"); }
        ,   [&](Type_1_1_1)    { rec("1_1_1"); }
        ,   [&](Type_1_1)      { rec("1_1"); }
        ,   [&](Type_1_2)      { rec("1_2"); }
        ,   [&](Type_1)        { rec("1"); }
        ,   [&](Type_2_1)      { rec("2_1"); }
        ,   [&](Type_2_2)      { rec("2_2"); }
        ,   [&](Type_2)        { rec("2"); }
        ,   [&]{ rec("fail"); }
        );
    }

    EXPECT_EQ(expected_matches, matches);
}

//----------------------------------------------------------------------------------------------------------------------

TEST_F(TypeHierarchyTest, match_matching_hierachies)
{
    std::vector<Root*>       ptrs             = {&x_1, &x_1_1, &x_1_2, &x_1_1_1, &x_1_1_2, &x_2, &x_2_1, &x_2_2};
    std::vector<std::string> expected_matches = {  "1",  "1_1",  "1_2",  "1_1_1",  "1_1_2",  "2",  "2_1",  "2_2"};
    std::vector<std::string> matches;

    auto rec = [&matches](std::string s) { matches.push_back(s); };

    for (Root* p : ptrs)
    {
        ni::match(*p)
        (   [&](Type_1& r){
                ni::match(r)
                (   [&](Type_1_1&){
                        ni::match(r)
                        (   [&](Type_1_1_1&){ rec("1_1_1"); }
                        ,   [&](Type_1_1_2&){ rec("1_1_2"); }
                        ,   [&]{ rec("1_1"); }
                        );
                    }
                ,   [&](Type_1_2&){ rec("1_2"); }
                ,   [&]{ rec("1"); }
                );
            }
        ,   [&](Type_2& r){
                ni::match(r)
                (   [&](Type_2_1&){ rec("2_1"); }
                ,   [&](Type_2_2&){ rec("2_2"); }
                ,   [&]{ rec("2"); }
                );
            }
        ,   [&]{ rec("fail"); }
        );
    }

    EXPECT_EQ(expected_matches, matches);
}

//----------------------------------------------------------------------------------------------------------------------

TEST_F(TypeHierarchyTest, trying_to_cast_across_hierarchies_must_fail_at_compile_time)
{
    struct OtherHierarchyBase {};
    using OtherRoot = ni::type_hierarchy::from_base<OtherHierarchyBase>;
    struct OtherType_1 : ni::sub_type<OtherType_1, OtherRoot> {};

    OtherType_1 ot1;
    OtherRoot& other_r = ot1;
    Root& test_r = x_1;

    EXPECT_TRUE( ni::convertible_to<OtherType_1>(other_r) );
    EXPECT_FALSE( ni::convertible_to<Type_1>(other_r) );
    EXPECT_FALSE( ni::convertible_to<OtherType_1>(test_r) );
}

//----------------------------------------------------------------------------------------------------------------------

TEST_F(TypeHierarchyTest, very_deep_hierarchy)
{
    struct DeepHierarchyBase {};

    using DeepRoot = ni::type_hierarchy::from_base<DeepHierarchyBase, 2, 2, 2, 2, 2, 2, 2, 2>;

    struct NI_SUB_TYPE( Deep_1, DeepRoot ) {};
    struct NI_SUB_TYPE( Deep_2, Deep_1   ) {};
    struct NI_SUB_TYPE( Deep_3, Deep_2   ) {};
    struct NI_SUB_TYPE( Deep_4, Deep_3   ) {};
    struct NI_SUB_TYPE( Deep_5, Deep_4   ) {};
    struct NI_SUB_TYPE( Deep_6, Deep_5   ) {};
    struct NI_SUB_TYPE( Deep_7, Deep_6   ) {};
    struct NI_SUB_TYPE( Deep_8, Deep_7   ) {};

    DeepRoot* r;
    Deep_1 d1;
    Deep_2 d2;
    Deep_3 d3;
    Deep_8 d8;

    r = &d1;
    EXPECT_TRUE( ni::convertible_to<Deep_1>(*r) );

    r = &d2;
    EXPECT_TRUE( ni::convertible_to<Deep_1>(*r) );
    EXPECT_TRUE( ni::convertible_to<Deep_2>(*r) );

    r = &d3;
    EXPECT_TRUE( ni::convertible_to<Deep_1>(*r) );
    EXPECT_TRUE( ni::convertible_to<Deep_2>(*r) );
    EXPECT_TRUE( ni::convertible_to<Deep_3>(*r) );

    r = &d8;
    EXPECT_TRUE( ni::convertible_to<Deep_1>(*r) );
    EXPECT_TRUE( ni::convertible_to<Deep_2>(*r) );
    EXPECT_TRUE( ni::convertible_to<Deep_3>(*r) );
    EXPECT_TRUE( ni::convertible_to<Deep_5>(*r) );
    EXPECT_TRUE( ni::convertible_to<Deep_6>(*r) );
    EXPECT_TRUE( ni::convertible_to<Deep_7>(*r) );
    EXPECT_TRUE( ni::convertible_to<Deep_8>(*r) );
}

//----------------------------------------------------------------------------------------------------------------------

TEST_F(TypeHierarchyTest, for_each_hierarchy_ids_must_start_counting_at_one)
{
    struct HierarchyBase1 {};
    struct HierarchyBase2 {};

    using Root1 = ni::type_hierarchy::from_base<HierarchyBase1>;
    using Root2 = ni::type_hierarchy::from_base<HierarchyBase2>;

    struct T1_L1 : ni::sub_type<T1_L1, Root1> {};
    struct T1_L2 : ni::sub_type<T1_L2, T1_L1> {};

    struct T2_L1 : ni::sub_type<T2_L1, Root2> {};
    struct T2_L2 : ni::sub_type<T2_L2, T2_L1> {};

    T1_L1  t1_l1;
    T1_L2  t1_l2;
    T2_L1  t2_l1;
    T2_L2  t2_l2;

    EXPECT_EQ( 1u, t1_l1.type_hierarchy_id__() );
    EXPECT_EQ( t1_l1.type_hierarchy_id__(), t2_l1.type_hierarchy_id__() );
    EXPECT_EQ( t1_l2.type_hierarchy_id__(), t2_l2.type_hierarchy_id__() );
}

//----------------------------------------------------------------------------------------------------------------------

TEST_F(TypeHierarchyTest, id_type_matches_provided_bits_per_level)
{
    struct Base {};

    using DefaultRoot = ni::type_hierarchy::from_base<Base>;
    struct DefaultType : ni::sub_type<DefaultType, DefaultRoot> {};
    DefaultType t;
    EXPECT_LE( 4u, sizeof(t.type_hierarchy_id__()) );

    using Root2x4 = ni::type_hierarchy::from_base<Base, 4, 4>;
    struct Type2x4 : ni::sub_type<DefaultType, Root2x4> {};
    Type2x4 t2x4;
    EXPECT_LE( 1u, sizeof(t2x4.type_hierarchy_id__()) );

    using Root4x4 = ni::type_hierarchy::from_base<Base, 4, 4, 4, 4>;
    struct Type4x4 : ni::sub_type<DefaultType, Root4x4> {};
    Type4x4 t4x4;
    EXPECT_LE( 2u, sizeof(t4x4.type_hierarchy_id__()) );

    using Root4x8 = ni::type_hierarchy::from_base<Base, 8, 8, 8, 8>;
    struct Type4x8 : ni::sub_type<DefaultType, Root4x8> {};
    Type4x8 t4x8;
    EXPECT_LE( 4u, sizeof(t4x8.type_hierarchy_id__()) );

    using Root4x16 = ni::type_hierarchy::from_base<Base, 16, 16, 16, 16>;
    struct Type4x16 : ni::sub_type<DefaultType, Root4x16> {};
    Type4x16 t4x16;
    EXPECT_LE( 8u, sizeof(t4x16.type_hierarchy_id__()) );
}
