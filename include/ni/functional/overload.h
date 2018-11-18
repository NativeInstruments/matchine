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
//!  Overloading lambdas and function objects to do basic pattern matching on types and create inline visitors for
//!  variant types.
//!
//!  Example
//!  ```
//!     std::variant<int, std::string, bool, double>  var;
//!
//!     auto visitor = ni::overload
//!     (  [](int n)         { std::cout << n << std::endl; }
//!     ,  [](std::string s) { std::cout << s << std::endl; }
//!     ,  [](auto x)        { std::cout << "default: " << x << std::endl; }
//!     );
//!
//!     var = 1337;
//!     std::visit(visitor, var);
//!
//!     var = std::string("Leet");
//!     std::visit(visitor, var);
//!
//!     var = true;
//!     std::visit(visitor, var);
//!
//!     var = 3.14;
//!     std::visit(visitor, var);
//!   ```
//!
//!  core idea taken from: https://stackoverflow.com/questions/7867555/best-way-to-do-variant-visitation-with-lambdas
//!  and the std proposal P0051R2: http://www.open-std.org/jtc1/sc22/wg21/docs
//!
//!---------------------------------------------------------------------------------------------------------------------


#pragma once

#include <utility>
#include <type_traits>

namespace ni
{
    template <typename... Lambdas>
    struct overloaded
    {};


    template <typename Lambda1, typename... Lambdas>
    struct overloaded<Lambda1, Lambdas...> 
    :   public Lambda1
    ,   public overloaded<Lambdas...>
    {
        using Lambda1::operator();
        using overloaded<Lambdas...>::operator();

        template <typename UniLambda1, typename... UniLambdas>
        overloaded(UniLambda1&& l1, UniLambdas&&... lambdas) 
        :  Lambda1{std::forward<UniLambda1>(l1)}
        ,  overloaded<Lambdas...>{std::forward<UniLambdas>(lambdas)...}
        {}
    };


    template <typename Lambda1>
    struct overloaded<Lambda1>
    :   public Lambda1
    {
        using Lambda1::operator();

        template <typename UniLambda1, typename =
            std::enable_if_t<std::is_same<Lambda1,std::decay_t<UniLambda1>>::value> >
        overloaded(UniLambda1&& l1)
        :  Lambda1{std::forward<UniLambda1>(l1)}
        {}
    };


    /* C++17 TODO
    template <typename... Lambdas>
    struct overloaded : public Lambdas...
    {
        using Lambdas::operator()...;

        template <typename... UniLambdas>  // enable_if_t< is_same_v<Lambdas, std::decay_t<UniLambdas>> && ... >
        overloaded(UniLambdas&&... lambdas) : Lambdas(std::forward<UniLambdas>(lambdas))... {}
    };
    */


    namespace detail
    {
        template <typename Function>
        struct free_function_wrapper
        {
            template <typename F>
            static decltype(auto) apply(F&& f) { return std::forward<F>(f); }
        };

        template <typename R, typename... Args>
        struct free_function_wrapper< R (*)(Args...) >
        {
            template <typename F>
            static auto apply(F* f)
            {
                return [f](Args... args)
                {
                    return f(std::forward<Args>(args)...);
                };
            }
        };


        template <typename Function>
        decltype(auto) wrap_free_function(Function&& f)
        {
            return free_function_wrapper<std::decay_t<Function>>::apply( std::forward<Function>(f) );
        }

        template <typename... Lambdas>
        auto overload(Lambdas&&... lambdas) -> overloaded<std::decay_t<Lambdas>...>
        {
            return { wrap_free_function(std::forward<Lambdas>(lambdas))... };
        }

    }


    template <typename... Lambdas>
    decltype(auto) overload(Lambdas&&... lambdas)
    {
        return detail::overload( detail::wrap_free_function(std::forward<Lambdas>(lambdas))... );
    }
}
