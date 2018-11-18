//
// MIT License
//
// Copyright © 2018
// Native Instruments
//
// For more detailed information, please read the LICENSE in the root directory.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - >8

#include <ni/functional/overload.h>
#include <boost/variant.hpp>
#include <gtest/gtest.h>
#include <string>
#include <sstream>

//----------------------------------------------------------------------------------------------------------------------

TEST( ni_overload, empty_overload_can_act_as_unit_type )
{
    ni::overload();
}

//----------------------------------------------------------------------------------------------------------------------

TEST( ni_overload, basic_lambda_overload )
{
    int    i = 0;
    double d = 0.;

    auto f = ni::overload
    (   [&i](int x)     { i = x; }
    ,   [&d](double x)  { d = x; }
    );

    f(7357);
    f(13.37);

    EXPECT_EQ(7357,  i);
    EXPECT_EQ(13.37, d);
}

//----------------------------------------------------------------------------------------------------------------------

TEST( ni_overload, lvalues_must_be_assignable_through_overloaded_onbject )
{
    int    i = 0;
    double d = 0.;

    auto f = ni::overload
    (   [](int& x)     { x = 2; }
    ,   [](double& x)  { x = 5.0; }
    );

    f(i);
    f(d);

    EXPECT_EQ(2,  i);
    EXPECT_EQ(5.0, d);
}

//----------------------------------------------------------------------------------------------------------------------

TEST( ni_overload, overload_on_one_of_two_arguments )
{
    auto f = ni::overload
    (   [](std::ostream& os, int x)          { os << x; }
    ,   [](std::ostream& os, std::string s)  { os << s; }
    );

    std::stringstream ss;

    f(ss, 1337);
    f(ss,"TEST");

    EXPECT_EQ("1337TEST", ss.str());
}

//----------------------------------------------------------------------------------------------------------------------

TEST( ni_overload, basic_function_object_overload )
{
    struct I { auto operator()(int x)    const { return x+1;  }}  fi;
    struct D { auto operator()(double x) const { return x+2.; }}  fd;

    auto f = ni::overload(fi,fd);

    EXPECT_EQ( 7357  + 1  , f(7357)  );
    EXPECT_EQ( 7357. + 2. , f(7357.) );
}

//----------------------------------------------------------------------------------------------------------------------

TEST( ni_overload, calling_overloads_with_heterogenous_return_types_will_return_correct_type )
{
    auto f = ni::overload
    (   [](int x)     { return x; }
    ,   [](double x)  { return x; }
    );

    EXPECT_TRUE(( std::is_same<int    , decltype(f(0 ))>::value ));
    EXPECT_TRUE(( std::is_same<double , decltype(f(0.))>::value ));
}

//----------------------------------------------------------------------------------------------------------------------

TEST( ni_overload, derived_classes_via_base_reference )
{
    struct Base             { virtual const char* get() const          { return "base"; } };
    struct Derived : Base   {         const char* get() const override { return "virtual derived"; } };

    auto f = ni::overload
    (   [](Base& o)   { return o.get(); }
    ,   [](Derived&)  { return "derived"; }
    );

    Base     b;
    Derived  d;
    Base&    brb = b;
    Base&    brd = d;

    EXPECT_STREQ("base"            , f(b) );
    EXPECT_STREQ("derived"         , f(d) );
    EXPECT_STREQ("base"            , f(brb) );
    EXPECT_STREQ("virtual derived" , f(brd) );
}

//----------------------------------------------------------------------------------------------------------------------

TEST( ni_overload, overload_works_as_boost_variant_visitor )
{
    boost::variant<int, std::string, bool, double, float>  var;

    auto visitor = ni::overload
    (  [](int)         { return 0; }
    ,  [](std::string) { return 1; }
    ,  [](bool)        { return 2; }
    ,  [](auto)        { return 3; }     // default: all other types, here double and float
    );

    var = 1337;
    EXPECT_EQ(0, boost::apply_visitor(visitor, var));

    var = std::string("Leet");
    EXPECT_EQ(1, boost::apply_visitor(visitor, var));

    var = true;
    EXPECT_EQ(2, boost::apply_visitor(visitor, var));

    var = 0.;
    EXPECT_EQ(3, boost::apply_visitor(visitor, var));

    var = 0.f;
    EXPECT_EQ(3, boost::apply_visitor(visitor, var));
}

//----------------------------------------------------------------------------------------------------------------------

TEST( ni_overload, overload_free_functions )
{
    using int_fn_t  = int(*)(int);
    using bool_fn_t = int(*)(bool);

    int_fn_t  fi = [](int)  { return 1; };
    bool_fn_t fb = [](bool) { return 2; };

    auto f = ni::overload(fi,fb);

    EXPECT_EQ(1, f(int{}));
    EXPECT_EQ(2, f(bool{}));
}
