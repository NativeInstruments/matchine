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
#include <string>
#include <iostream>


int main()
{
    auto print = [](std::any const& a) {
        auto x = ni::match(a)
        (   [](int i)         { return std::to_string(i); }
        ,   [](std::string s) { return s; }
        ,   [](char const* s) { return std::string(s); }
        );
        if (x)
            std::cout << *x << std::endl;
        else
            std::cout << "<not supported>" << std::endl;
    };

    std::any
    a = 1337;
    print(a);
    a = std::string("Leet");
    print(a);
    a = "C Leet";
    print(a);
    a = 3.1415;
    print(a);

    /* This will print:

        1337
        Leet
        C Leet
        <not supported>
    */
}
