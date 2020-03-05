# Concepts Overview 

## Introduction

Concepts is a new language feature fully supported in C++20
that aims to simply generic programming and equip users with type safety.
The following is a quick overview of concepts, mostly adopted and inspired by the [concepts and constraints documentation page](https://en.cppreference.com/w/cpp/language/constraints).

## Writing Concepts

### type_traits

One preliminary thing has to be introduced and that's `<type_traits>` header.
Take a look at [type_traits](https://en.cppreference.com/w/cpp/header/type_traits) 
for a lot of cool metaprogramming tools that you can use for this assignment 
and to understand existing implementations of concepts.
A lot of low-level concepts actually end up wrapping some of these tools.
Moreover, most of the concepts are defined using these tools.

Here's an example of a tool in `<type_traits>` that comes up frequently:
```cpp
template< class T >
using remove_reference_t = typename remove_reference<T>::type;  // since C++14
```

Note that `remove_reference` is actually a class template. 
Think of `remove_reference` as a mapping from a template parameter `T` to some other type,
which gets stored as a member alias `type`.
It is specially written such that it will give you whatever type you passed in 
minus the `&`'s essentially:
```cpp
std::remove_reference_t<int> x; 
std::remove_reference_t<int&> y; 
std::remove_reference_t<int&&> z;
// x, y, and z are all of type int
```

`remove_reference_t` was an example of mapping types to types, but we can also map types to compile-time known values. Here's an example 
```cpp
template <class T>
inline constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value; // since C++17
```

Again, `is_lvalue_reference` is a class template and it can be thought of as a mapping
from a type `T` to some (compile-known) value, which gets stored as a static constexpr bool member
called `value`.
It is specially written such that it will evaluate to true so long as `T` is some lvalue reference.
As an example, the following behavior holds:
```cpp
std::is_lvalue_reference<int>;  // evaluates to false - not a reference type
std::is_lvalue_reference<int&>; // evaluates to true  - is lvalue reference type
std::is_lvalue_reference<int&>; // evaluates to false - is rvalue reference type
```

These patterns are pervasive in metaprogramming; pretty much all the stuff in `type_traits` is defined in this way.

### Pre-defined Concepts 

Many useful pre-defined concepts have already been defined in `<concepts>` header,
which are only available starting from gcc-10.
One can view documentation pages for these concepts in [this page](https://en.cppreference.com/w/cpp/header/concepts).
You may copy and paste the possible implementations for these concepts into your code to build on top of them.

Looking at their implementation is one of the best ways to learn how to use concepts!
Here is a simplified implementation of `equality_comparable`, which we also saw in class, of the exposition in [this page](https://en.cppreference.com/w/cpp/concepts/equality_comparable):
```cpp
template<class T>
concept equality_comparable = 
    requires(const std::remove_reference_t<T>& t,
             const std::remove_reference_t<T>& u) {
        { t == u } -> std::boolean;
        { t != u } -> std::boolean;
        { u == t } -> std::boolean;
        { u != t } -> std::boolean;
    };
```

We now know what `remove_reference_t` does (check [this](#type_traits))!
It's just to ensure that whatever gets passed in as `T`, 
we want to remove any reference and then add an lvalue reference, 
such that `std::remove_reference_t<T>&` is truly an lvalue reference type.

The requires-expression can be used to specify variable names with certain types,
and use them in expressions like `{ t == u }`.
Note that none of these variables ever get allocated and are purely there to see if
syntactically the expressions are valid!
Lastly, the return-type-constraint (stuff followed by `->`) must be a concept starting from C++20.
In TS version, they allow this constraint to be types, i.e. the following was allowed:
```cpp
{t == u} -> bool;
```
but was removed from the standard in C++20.
Note also that `std::boolean` is a pre-defined concept in `<concepts>`.
It takes in a single template parameter.
When specifying return-type-constraint, the compiler deduces the first parameter from the return type of the expression.

### User-defined Concepts

Let's try implementing a couple of concepts.
One example I just concocted is Incrementable.
A type T satisfies Incrementable if the following hold:
let `x` be an object of type `T` and 
- x++ compiles and returns something that is convertible to T
- ++x compiles and returns something that is T&

Here is a possible implementation of the new concept Incrementable:
```cpp
template <class T>
concept Incrementable = 
    requires(T x) {
        { x++ } -> std::convertible_to<T>;
        { ++x } -> std::same_as< std::add_lvalue_reference_t<
                                 std::remove_reference_t<T> > >;
    };
```

Turns out, `<concepts>` implements the concept `convertible_to` and `same_as`,
and `<type_traits>` contains `add_lvalue_reference_t` and `remove_reference_t`.
You can read more about them there.

We can also define Decrementable in a similar way:
```cpp
template <class T>
concept Decrementable = 
    requires(T x) {
        { x-- } -> std::convertible_to<T>;
        { --x } -> std::same_as< std::add_lvalue_reference_t<
                                 std::remove_reference_t<T> > >;
    };
```

Now you can combine these building blocks to create a more complex concept.
Let's define a type to satisfy the concept Crementable if it satisfies Incrementable
and Decrementable.
The following is a possible implementation:
```cpp
template <class T>
concept Crementable = Incrementable<T> && Decrementable<T>;
```

#### How do we use these concepts?

Let's write a couple of stupid, but instructive functions.
We will first write a double-incrementing function using both prefix and postfix operator++.
As shown in lecture, we'll use the lifting method to motivate the need for the above concepts.

Consider the following function that compiles even with pre-C++11 compiler.
```cpp
int& double_increment(int& x) 
{T y = x++; return ++x;}
```
It first postfix-increments `x` and stores the result into `y`, which is of type `T`.
And then we return the result of `++x`, hence double-incrementing.

Of course, this wrapper actually applies more generally to other types such as `char, double, Complex`, etc.
We can generalize this by using templates as follows:
```cpp
template <class T>
T& double_increment(T& x)
{T y = x++; return ++x;}
```

The problem now is that `T` is way too general - 
what if `x++` or `++x` doesn't even compile for some types?
What if `x++` returns something so crazy that it cannot even be converted to type `T`?
The last step is to _constrain_ the type `T` such that both expressions are valid,
`x++` returns something that can be converted to `T`,
and `++x` returns something of type `T&`.
This is precisely our Incrementable concept.
Using concepts, we can fully generalize the function as such:

```cpp
template <Incrementable T>
T& increment(T& x)
{T y = x++; return ++x;}
```

The test code is located in `src` directory - feel free to play around with it!

----

Hopefully, this was helpful in getting you started with the assignment and understanding what concepts is all about! :)
