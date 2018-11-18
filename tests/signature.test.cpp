//
// MIT License
//
// Copyright © 2018
// Native Instruments
//
// For more detailed information, please read the LICENSE in the root directory.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - >8

#include <ni/functional/signature.h>

#include <gtest/gtest.h>
#include <type_traits>

//----------------------------------------------------------------------------------------------------------------------

namespace ni_functional_signature_test_detail
{
    struct R {};
    struct A {};

    R     R_int(A,int);
    auto  auto_int_to_int(int) -> int { return 42; }
}

TEST( ni_functional_signature_test , plain_functions )
{ 
    using namespace std;
    using namespace ni_functional_signature_test_detail;

    EXPECT_TRUE(( is_same< ni::signature< decltype(R_int) >::type              , R(A,int) >   ::value ));

    EXPECT_TRUE(( is_same< ni::signature< decltype(R_int) >::result_type       , R >   ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(R_int) >::argument<0>::type , A >   ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(R_int) >::argument<1>::type , int > ::value ));

    EXPECT_TRUE(( is_same< ni::signature< decltype(auto_int_to_int) >::result_type       , int > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(auto_int_to_int) >::argument<0>::type , int > ::value ));
}

//----------------------------------------------------------------------------------------------------------------------

TEST( ni_functional_signature_test , pointer_to_function )
{ 
    using ptr_void_int = void (*)(int);

    EXPECT_TRUE(( std::is_same< ni::signature< ptr_void_int >::type              , void(int) > ::value ));
    EXPECT_TRUE(( std::is_same< ni::signature< ptr_void_int >::argument<0>::type , int       > ::value ));
}

//----------------------------------------------------------------------------------------------------------------------

TEST( ni_functional_signature_test , reference_to_function )
{ 
    using ref_void_int = void (&)(int);

    EXPECT_TRUE(( std::is_same< ni::signature< ref_void_int >::type              , void(int) > ::value ));
    EXPECT_TRUE(( std::is_same< ni::signature< ref_void_int >::argument<0>::type , int       > ::value ));
}

//----------------------------------------------------------------------------------------------------------------------

TEST( ni_functional_signature_test , lambdas )
{
    using namespace std;

    auto lambda_void_void = []{};
    EXPECT_TRUE(( is_same< ni::signature< decltype(lambda_void_void) >::type        , void(void) > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(lambda_void_void) >::result_type , void       > ::value ));

    auto lamda_int_float = [](float) mutable { return 3; };
    EXPECT_TRUE(( is_same< ni::signature< decltype(lamda_int_float) >::type              , int(float) > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(lamda_int_float) >::result_type       , int        > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(lamda_int_float) >::argument<0>::type , float      > ::value ));
}

//----------------------------------------------------------------------------------------------------------------------

TEST( ni_functional_signature_test , function_objects )
{
    using namespace std;

    struct { int operator()() { return 0; } }                 funob;
    struct { int operator()() const { return 0; } }           funob_const;
    struct { int operator()() volatile { return 0; } }        funob_volatile;
    struct { int operator()() const volatile { return 0; } }  funob_const_volatile;

    EXPECT_TRUE(( is_same< ni::signature< decltype(funob) >::type                , int(void) > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(funob_const) >::type          , int(void) > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(funob_volatile) >::type       , int(void) > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(funob_const_volatile) >::type , int(void) > ::value ));

    // references to objects

    auto& r_funob                 = funob;
    auto& r_funob_const           = funob_const;
    auto& r_funob_volatile        = funob_volatile;
    auto& r_funob_const_volatile  = funob_const_volatile;

    std::ignore = r_funob;
    std::ignore = r_funob_const;
    std::ignore = r_funob_volatile;
    std::ignore = r_funob_const_volatile;

    EXPECT_TRUE(( is_same< ni::signature< decltype(r_funob) >::type                , int(void) > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(r_funob_const) >::type          , int(void) > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(r_funob_volatile) >::type       , int(void) > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(r_funob_const_volatile) >::type , int(void) > ::value ));

    // const references to objects

    auto const& cr_funob                 = funob;
    auto const& cr_funob_const           = funob_const;
    auto const& cr_funob_volatile        = funob_volatile;
    auto const& cr_funob_const_volatile  = funob_const_volatile;

    std::ignore = cr_funob;
    std::ignore = cr_funob_const;
    std::ignore = cr_funob_volatile;
    std::ignore = cr_funob_const_volatile;

    EXPECT_TRUE(( is_same< ni::signature< decltype(cr_funob) >::type                , int(void) > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(cr_funob_const) >::type          , int(void) > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(cr_funob_volatile) >::type       , int(void) > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(cr_funob_const_volatile) >::type , int(void) > ::value ));

    // const volatile objects

    auto const volatile cv_funob                 = funob;
    auto const volatile cv_funob_const           = funob_const;
    auto const volatile cv_funob_volatile        = funob_volatile;
    auto const volatile cv_funob_const_volatile  = funob_const_volatile;

    std::ignore = cv_funob;
    std::ignore = cv_funob_const;
    std::ignore = cv_funob_volatile;
    std::ignore = cv_funob_const_volatile;

    EXPECT_TRUE(( is_same< ni::signature< decltype(cv_funob) >::type                , int(void) > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(cv_funob_const) >::type          , int(void) > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(cv_funob_volatile) >::type       , int(void) > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(cv_funob_const_volatile) >::type , int(void) > ::value ));

    // r-value objects

    EXPECT_TRUE(( is_same< ni::signature< decltype(std::move(funob)) >::type                , int(void) > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(std::move(funob_const)) >::type          , int(void) > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(std::move(funob_volatile)) >::type       , int(void) > ::value ));
    EXPECT_TRUE(( is_same< ni::signature< decltype(std::move(funob_const_volatile)) >::type , int(void) > ::value ));
}
