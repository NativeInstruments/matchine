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
#include <memory>
#include <vector>
#include <string>
#include <iostream>


struct Event { virtual ~Event(){} };
struct MouseEvent : Event {
    int x, y;
    MouseEvent(int x_, int y_) : x{x_}, y{y_} {}
};
struct KeyEvent : Event {
    char key;
    KeyEvent(char k) : key{k} {}
};
struct MyEvent : Event {};

int main()
{
    using namespace std;

    vector<shared_ptr<Event>> events =
    {   make_shared<MouseEvent>(13, 37)
    ,   make_shared<KeyEvent>('\15')
    ,   make_shared<KeyEvent>('%')
    ,   make_shared<MyEvent>()
    };

    for (auto const& event : events)
    {
        auto deref = [](auto const& p) -> auto& { return *p; };
        auto s = ni::match(deref(event))
        (   [](MouseEvent const& m) {
                return to_string(m.x) + ", " + to_string(m.y); }
        ,   [](KeyEvent const& k) { return to_string(k.key); }
        ,   ni::otherwise(string("<unknown type>"))
        );
        std::cout << s << std::endl;
    }
}
