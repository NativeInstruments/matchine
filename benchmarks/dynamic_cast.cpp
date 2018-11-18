//
// MIT License
//
// Copyright © 2018
// Native Instruments
//
// For more detailed information, please read the LICENSE in the root directory.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - >8

#define NONIUS_RUNNER
#include <nonius/nonius.h++>
#include <nonius/main.h++>

#include <ni/benchmark/escape_from_optimizer.h>
#include <ni/functional/match.h>
#include <ni/type_hierarchy.h>


struct L0 { virtual ~L0(){} };
struct L1a : L0 {};
struct L1b : L0 {};
struct L2a : L1a {};
struct L2b : L1a {};
struct L3aa : L2a {};

L1a  l1a;
L2a  l2a;
L3aa l3aa;


NONIUS_BENCHMARK("virtual match level 0 -> level 1", []
{
   L0 const* p = &l1a;
   escape_from_optimizer(p);
   escape_from_optimizer(ni::match(*p)([](L1a const&){}));
});

NONIUS_BENCHMARK("virtual fail level 0 -> level 1", []
{
   L0 const* p = &l1a;
   escape_from_optimizer(p);
   escape_from_optimizer(ni::match(*p)([](L1b const&){}));
});

NONIUS_BENCHMARK("virtual match level 0 -> level 2", []
{
   L0 const* p = &l2a;
   escape_from_optimizer(p);
   escape_from_optimizer(ni::match(*p)([](L2a const&){}));
});

NONIUS_BENCHMARK("virtual fail level 0 -> level 2", []
{
   L0 const* p = &l2a;
   escape_from_optimizer(p);
   escape_from_optimizer(ni::match(*p)([](L2b const&){}));
});

NONIUS_BENCHMARK("virtual match level 1 -> level 2", []
{
   L1a const* p = &l2a;
   escape_from_optimizer(p);
   escape_from_optimizer(ni::match(*p)([](L2a const&){}));
});

NONIUS_BENCHMARK("virtual fail level 1 -> level 2", []
{
   L1a const* p = &l2a;
   escape_from_optimizer(p);
   escape_from_optimizer(ni::match(*p)([](L2b const&){}));
});

NONIUS_BENCHMARK("virtual match level 2 -> level 1", []
{
   L2a const* p = &l2a;
   escape_from_optimizer(p);
   escape_from_optimizer(ni::match(*p)([](L1a const&){}));
});

NONIUS_BENCHMARK("virtual fail level 2 -> level 1", []
{
   L2a const* p = &l2a;
   escape_from_optimizer(p);
   escape_from_optimizer(ni::match(*p)([](L1b const&){}));
});



struct Root {};

using H0 = ni::type_hierarchy::from_base<Root>;
struct H1a : ni::sub_type<H1a, H0> {};
struct H1b : ni::sub_type<H1b, H0> {};
struct H2a : ni::sub_type<H2a, H1a> {};
struct H2b : ni::sub_type<H2b, H1a> {};
struct H3aa : ni::sub_type<H3aa, H2a> {};

H1a  h1a;
H2a  h2a;
H3aa h3aa;

NONIUS_BENCHMARK("hierarchy match level 0 -> level 1", []
{
   H0 const* p = &h1a;
   escape_from_optimizer(p);
   escape_from_optimizer(ni::match(*p)([](H1a const&){}));
});

NONIUS_BENCHMARK("hierarchy fail level 0 -> level 1", []
{
   H0 const* p = &h1a;
   escape_from_optimizer(p);
   escape_from_optimizer(ni::match(*p)([](H1b const&){}));
});

NONIUS_BENCHMARK("hierarchy match level 0 -> level 2", []
{
   H0 const* p = &h2a;
   escape_from_optimizer(p);
   escape_from_optimizer(ni::match(*p)([](H2a const&){}));
});

NONIUS_BENCHMARK("hierarchy fail level 0 -> level 2", []
{
   H0 const* p = &h2a;
   escape_from_optimizer(p);
   escape_from_optimizer(ni::match(*p)([](H2b const&){}));
});

NONIUS_BENCHMARK("hierarchy match level 1 -> level 2", []
{
   H1a const* p = &h2a;
   escape_from_optimizer(p);
   escape_from_optimizer(ni::match(*p)([](H2a const&){}));
});

NONIUS_BENCHMARK("hierarchy fail level 1 -> level 2", []
{
   H1a const* p = &h2a;
   escape_from_optimizer(p);
   escape_from_optimizer(ni::match(*p)([](H2b const&){}));
});

NONIUS_BENCHMARK("hierarchy match level 2 -> level 1", []
{
   H2a const* p = &h2a;
   escape_from_optimizer(p);
   escape_from_optimizer(ni::match(*p)([](H1a const&){}));
});


//  This case currently does not work, since there is a certain compile time
//  safety of the hierarchy & match framework. This would only work if dyn_cast
//  would use reinterpret_cast instead of static_cast. Leaving this in for now,
//  in case future scenario pop up where this is required.
//
//  NONIUS_BENCHMARK("hierarchy fail level 2 -> level 1", []
//  {
//     H2a const* p = &h2a;
//     escape_from_optimizer(p);
//     escape_from_optimizer(ni::match(*p)([](H1b const&){}));
//  });
