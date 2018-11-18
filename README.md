# MATCHINE

**Matchine** is a small collection of C++ tools developed at [Native Instruments](https://www.native-instruments.com) for more fun with sum types, such variants, inheritance hierarchies, etc.

The three main tools are

* `ni::overload` allows to create a single object that is a set of overloaded functions (free, lambdas, functors) following the C++ proposal [P0051R2](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0051r2.pdf). Its main application is to construct visitors inplace for `std::visit` but it's not limited to that.

* `ni::match` and `ni::matcher` allows to have a similar syntax of `std::visit` and `ni::overload` for open sum types, e.g. std::any, inheritance hierarchies, etc. It provides a simple pattern matching mechanism for types based on the signature of the provided functions.

* `ni::type_hierarchy` allows to build inheritance based polymorphic types without using the slow RTTI mechanism. This improves the performance of casts by an order of magnitude.


__Disclaimer.__ This documentation is work in progress, please check back soon for more details.

## Installation

The library is header only. So simply clone and go. A [`CMakeLists.txt`](CMakeLists.txt) is provided.



## Quick Introduction


### `ni::overload`

```c++
#include <ni/functional/overload.h>
#include <vector>
#include <variant>
#include <string>
#include <iostream>

struct MyType {};
using var_t = std::variant<double, int, MyType>;

int main()
{
    std::vector<var_t>  vars = { var_t{3.14}, var_t{1337}, var_t{MyType{}} };
    for (auto const& var : vars)
    {
        // instead of requiring to construct a custom function object type elsewhere
        // one is able to overload several lambdas bound into one object.
        auto visitor = ni::overload
        (   [](double x) { return "float: " + std::to_string(x); }
        ,   [](int n) { return "int: " + std::to_string(n); }
        ,   [](auto) { return std::string("<unknown type>"); }
        );
        std::cout << std::visit(visitor, var) << std::endl;
    }
}
```
This will print:
```
float: 3.140000
int: 1337
<unknown type>
```


### `ni::match`

Assume we have a polymorphic (i.e. sum) type, e.g. user input events:
```c++
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

```
Further we have a sequence of these events here simulated by putting different events into a vector:
```c++
using namespace std;

vector<shared_ptr<Event>> events =
{   make_shared<MouseEvent>(13, 37)
,   make_shared<KeyEvent>('k')
,   make_shared<MyEvent>()
};

```
`ni::match` calls the correct lambda depending on the detected event type:
```c++
for (auto const& event : events)
{
    auto s = ni::match(*event)
    (   [](MouseEvent const& m) {
            return to_string(m.x) + ", " + to_string(m.y); }
    ,   [](KeyEvent const& k) { return to_string(k.key); }
    ,   ni::otherwise(string("<unknown type>"))
    );
    std::cout << s << std::endl;
}
```

This will print:
```
13, 37
107
<unknown type>
```


### `ni::type_hierarchy`

`ni::type_hierarchy` allows to build inheritance based polymorphic types without using the slow RTTI mechanism. This improves the performance of casts by an order of magnitude.

Let's model user input events (key press, mouse move, etc.). A possible way to do this is to use inheritance and might look like this:
```c++
struct Event { virtual ~Event() = default; };

struct MouseEvent : Event {
    int x, y;
    MouseEvent(int x_, int y_) : x{x_}, y{y_} {}
}

struct MouseButtonDownEvent : MouseEvent {
    enum button_t { left, right };
    button_t button;
    MouseButtonDownEvent(int x_, int y_, button_t b)
    :   MouseEvent{x_, y_}
    ,   button{b}
    {}
}

// struct MouseButtonUpEvent : MouseEvent { ... }
// struct KeyPressEvent : Event { ... }
```
This approach has one major drawback: casting between the types uses RTTI, which is flexible, but due to that also slow because it has to handle many edge cases. If we limit ourselves to tree-like hierarchies the identification of types can be 

```c++
struct EventBase { ... };   // place to put common state shared by all objects.

// get the root type from the hierarchy which serves as reference/pointer to base class.
using Event = ni::type_hierarchy::from_base<EventBase>;

// derive custom types, the first template argument must always be the derived type
// itself. This is required due to the missing reflections in C++ and is used to
// assign a unique id to the new type.
struct MouseEvent : ni::sub_type<MouseEvent, Event> {
    int x, y;
    MouseEvent(int x_, int y_) : x{x_}, y{y_} {}
};
struct KeyEvent : ni::sub_type<KeyEvent, Event> {
    // ...
};

// 2nd level event
struct MouseButtonDownEvent : ni::sub_type<MouseButtonDownEvent, MouseEvent> {
    // ...
};

// etc..

```
See [benchmarks](benchmarks/README.md) for a speed comparison.



