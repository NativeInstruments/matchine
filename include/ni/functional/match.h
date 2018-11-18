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
//!
//!  \file
//!
//!  `ni::match` allows to do pattern matching on polymorphic types. `ni::match` takes one argument, which is the
//!   object that should be matched on and returns a function that takes a variadic list of functions that define
//!   the matching cases. The provided functions (mostly lambdas) must take exactly one argument which is the type
//!   of the specific case to match on.
//!
//!   All provided functions/lambdas must have the same return type. `ni::match` will return the value of the
//!   the lambda that got matched on wrapped inside an optional<>. If no type could be matched the optional will
//!   be empty.
//!
//!   If the result types are void `ni::match` will return a bool indicating a successful match.
//!
//!   It is possible to specify a custom result value for successful matches by adding a lambda without arguments
//!   that returns the value when called. It's also possible to use `ni::otherwise` to define the value. This is
//!   only provided as syntactic sugar. There may be only one default value per match!
//!
//!   Example
//!   ```
//!     struct event { virtual ~event() };
//!     struct mouse_up : event {};
//!     struct mouse_down : event {};
//!     struct mouse_drag : event {};
//!
//!     std::vector<shared_ptr<event>>  events = {…};
//!
//!     for (auto e : events)
//!         ni::match(*e)
//!         (   [this](mouse_up const& e)    { handle_mouse_up(e); }
//!         ,   [this](mouse_down const& e)  { handle_mouse_down(e); }
//!         ,   [this](mouse_drag const& e)  { handle_mouse_drag(e); }
//!         );
//!   ```
//!
//!   It is further possible to build a matcher and invoke it later using `ni::matcher`
//!
//!   Example
//!   ```
//!     auto my_matcher = ni::matcher
//!     (   [this](mouse_up const& e)    { handle_mouse_up(e); }
//!     ,   [this](mouse_down const& e)  { handle_mouse_down(e); }
//!     ,   [this](mouse_drag const& e)  { handle_mouse_drag(e); }
//!     );
//!     boost::for_each(events, my_matcher);
//!
//!   ```
//!
//!---------------------------------------------------------------------------------------------------------------------

#pragma once

#include <ni/functional/signature.h>
#include <ni/meta/fold_or.h>
#include <ni/meta/fold_add.h>
#include <ni/meta/try_catch.h>

#include <boost/optional.hpp>


namespace ni
{

    //  dyn_cast<> is a customization point for (open) sum types,
    //  e.g. polymorphic (virtual) classes, std/boost::any, custom implementation using some tag system, etc.
    //
    //  template <typename TargetType>
    //  TargetType* dyn_cast(CustomType* p) { return b->custom_cast<TargetType>(); }


    // this let's ADL kick in for the client code for chosing the right dyn_cast<>
    template <typename> void dyn_cast();



    namespace detail
    {
        // ADL can be quite weird. This SFINAE dispatch helps to find the right function if it exists.
        template <typename TargetType> struct target_type {};

        template <typename TargetType, typename SourceType>
        auto matcher_dyn_cast(meta::try_t, target_type<TargetType>, SourceType* p) -> decltype(dyn_cast<TargetType>(p))
        {
            return dyn_cast<TargetType>(p);
        }

        template <typename TargetType, typename SourceType>
        auto matcher_dyn_cast(meta::catch_t, target_type<TargetType>, SourceType* p) -> decltype(dynamic_cast<TargetType*>(p))
        {
            return dynamic_cast<TargetType*>(p);
        }

        // Some helper type trait

        template <typename Function, typename... Functions>
        struct result
        {
            using type = typename ni::signature<Function>::result_type;

            template <typename...> struct type_list {};

            template <typename F>
            using as_type = std::decay_t<type>;

            using all_results = type_list<typename ni::signature<Functions>::result_type...>;
            using expected_results = type_list<as_type<Functions>...>;

            static_assert( std::is_same< all_results, expected_results>::value
                         , "All functions must have the same result type.");

        };


        template <typename... Functions>
        struct result_type_info
        {
            static constexpr size_t sum_of_arguments = meta::fold_add_v<size_t, signature<Functions>::number_of_arguments...>;
            using wrapped_result_t = typename result<Functions...>::type;
            using result_t = std::conditional_t
            <   std::is_same<void,wrapped_result_t>::value
            ,   bool
            ,   std::conditional_t
                <   sum_of_arguments == size_t(sizeof...(Functions))
                ,   boost::optional<wrapped_result_t>
                ,   wrapped_result_t
                >
            >;
        };


        template <typename ResultType>
        struct invoker
        {
            template <typename Function>
            static decltype(auto) apply(Function& f)
            {
                return f();
            }

            template <typename Function, typename Arg>
            static decltype(auto) apply(Function& f, Arg& a)
            {
                return f(a);
            }
        };

        template <>
        struct invoker<void>
        {
            template <typename Function>
            static decltype(auto) apply(Function& f)
            {
                return f(), true;
            }

            template <typename Function, typename Arg>
            static decltype(auto) apply(Function& f, Arg& a)
            {
                return f(a), true;
            }
        };


        // Actual dispatcher. Possible optimization for long lists use hash-map for O(1) lookup

        template <typename ResultTypeInfo, typename Type, typename Lambda, typename... Lambdas>
        auto matcher_impl(Type* x, Lambda& l, Lambdas&... ls)
        -> std::enable_if_t< signature<Lambda>::number_of_arguments == 1, typename ResultTypeInfo::result_t>;


        template <typename ResultTypeInfo, typename Type>
        auto matcher_impl(Type*) -> typename ResultTypeInfo::result_t
        {
            return {};
        }

        template <typename ResultTypeInfo, typename Type, typename Lambda1, typename Lambda2, typename... Lambdas>
        auto matcher_impl(Type* x, Lambda1& l1, Lambda2& l2, Lambdas&... ls)
        -> std::enable_if_t< signature<Lambda1>::number_of_arguments == 0, typename ResultTypeInfo::result_t>
        {
            return matcher_impl<ResultTypeInfo>(x,l2,ls...,l1);
        }

        template <typename ResultTypeInfo, typename Type, typename Lambda, typename... Lambdas>
        auto matcher_impl(Type*, Lambda& l)
        -> std::enable_if_t< signature<Lambda>::number_of_arguments == 0, typename ResultTypeInfo::result_t>
        {
            return invoker<typename ResultTypeInfo::wrapped_result_t>::apply(l);
        }

        template <typename ResultTypeInfo, typename Type, typename Lambda, typename... Lambdas>
        auto matcher_impl(Type* x, Lambda& l, Lambdas&... ls)
        -> std::enable_if_t< signature<Lambda>::number_of_arguments == 1, typename ResultTypeInfo::result_t>
        {
            using target_t = std::remove_reference_t<typename signature<Lambda>::template argument<0>::type>;
            // TODO add const if Type has const
            if (auto* p = matcher_dyn_cast(meta::try_t{}, target_type<target_t>{}, x))
                return invoker<typename ResultTypeInfo::wrapped_result_t>::apply(l,*p);
            else
                return matcher_impl<ResultTypeInfo>(x,ls...);
        }
    }

    template <typename Value>
    auto otherwise(Value value)
    {
        return [value=std::move(value)]{ return value; };
    }


    template <typename... Lambdas>
    auto matcher(Lambdas&&... lambdas)
    {
        static_assert(
            meta::fold_and_v<
                (   (signature<Lambdas>::number_of_arguments == 0)
                or  (signature<Lambdas>::number_of_arguments == 1)
                )...
            >
            , "Can only match on lambdas with one argument."
        );

        using result_info_t = ::ni::detail::result_type_info<Lambdas...>;

        constexpr auto num_args = result_info_t::sum_of_arguments;
        static_assert(
            num_args == sizeof...(Lambdas) or num_args == sizeof...(Lambdas) - 1
            , "There can be only one default value defined per matcher."
        );

        return [=](auto& x) -> typename result_info_t::result_t
        {
            return ::ni::detail::matcher_impl<result_info_t>(&x, lambdas...);
        };
    }


    template <typename Type>
    auto match(Type& x)
    {
        return [&x](auto&&... lambdas)
        {
            return ::ni::matcher(std::forward<decltype(lambdas)>(lambdas)...)(x);
        };
    }

}