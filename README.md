# Concepts Overview 

## Introduction

Concepts is a new language feature fully supported in C++20
that aims to simply generic programming and equip users with type safety.
The following is a quick overview of concepts, mostly adopted and inspired by the [concepts and constraints documentation page](https://en.cppreference.com/w/cpp/language/constraints).

## Concepts

### Pre-defined Concepts 

Many useful pre-defined concepts have already been defined in `<concepts>` header,
which are only available starting from gcc-10.
One can view documentation pages for these concepts in [this page](https://en.cppreference.com/w/cpp/header/concepts).
You may copy and paste the possible implementations for these concepts into your code to build on top of them.

#### equality_comparable (simplified)

Looking at their implementation is one of the best ways to learn how to use concepts!
Here is a simplified implementation of `equality_comparable`, which we also saw in class, 
of the one in [this page](https://en.cppreference.com/w/cpp/concepts/equality_comparable):
```cpp
template<class T, class U = T>
concept equality_comparable = 
    requires(T t, U u) {
        { t == u } -> bool;
        { t != u } -> bool;
        { u == t } -> bool;
        { u != t } -> bool;
    };
```

What does this mean? `equality_comparable` is given two types in general, 
where the second type is by default the same as the first type if unspecified,
and checks that the expressions using `operator==` and `operator!=` on these types are valid
as well as return a value of type `bool`.

The requires-expression can be used to specify variable names with certain types,
and these variables can be used in expressions like `{ t == u }`.
Note that none of these variables ever get allocated and are purely there to see if
the expressions are syntactically valid!
Lastly, the return-type-constraint (stuff followed by `->`) must be a concept starting from C++20.
In TS version (experimental), they allow this constraint to be types, i.e. the following was allowed:
```cpp
{ t == u } -> bool;
```
but was *removed from the standard in C++20* (so this doesn't actually compile with C++20, 
but does with previous standards with `-fconcepts`).

#### equality_comparable (not so simplified)

Ok, the [previous section](#equality_comparable-simplified) was a bit simplified (**which is OK for homework**!),
but how do they do it in the standard library implementation?

Digging through the documentation, the following implementation is a lot closer to the actual one:

```cpp
template<class T, class U = T>
concept equality_comparable = 
    requires(const std::remove_reference_t<T>& t,
             const std::remove_reference_t<U>& u) {
        { t == u } -> std::boolean;
        { t != u } -> std::boolean;
        { u == t } -> std::boolean;
        { u != t } -> std::boolean;
    };
```

One preliminary thing has to be introduced and that's `<type_traits>` header.
Take a look at [type_traits](https://en.cppreference.com/w/cpp/header/type_traits) 
for a lot of cool metaprogramming tools that you can (but not required to) use for this assignment 
and to understand existing implementations of concepts.
A lot of low-level concepts actually end up wrapping some of these tools.
Moreover, most of the concepts are defined using these tools.

Here's an example of a tool in `<type_traits>` that comes up frequently, as we see above:
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

`remove_reference_t` is just being used in `equality_comparable`
to ensure that whatever gets passed in as `T` will be stripped of any references (&'s).
This way, `std::remove_reference_t<T>&` is truly an lvalue reference type.

Note also that `std::boolean` is a pre-defined concept in `<concepts>`.
It checks whether a type can be used in Boolean context.
It takes in a single template parameter, but note that we never specified the parameter, i.e. didn't do something like
```cpp
{ t == u } -> std::boolean</* something */>;
```
When specifying return-type-constraint, the compiler substitutes the first parameter with the return type of the expression.

### User-defined Concepts

Let's try implementing a couple of concepts.
One example I just concocted is `Incrementable`.
A type `T` satisfies `Incrementable` if the following hold:
let `x` be an object of type `T` and 
- `x++` compiles and returns something that is convertible to `T`
- `++x` compiles and returns something that is `T&`

Here is a possible implementation of the new concept `Incrementable`:
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
You can read more about them in the documentation page.

We can also define `Decrementable` in a similar way:
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
Let's define a type to satisfy the concept `Crementable` if it satisfies `Incrementable`
and `Decrementable`.
The following is a possible implementation:
```cpp
template <class T>
concept Crementable = Incrementable<T> && Decrementable<T>;
```

#### How do we use these concepts?

Let's write a couple of stupid, but instructive functions.
We will first write a double-incrementing function using both prefix and postfix `operator++`.
As shown in lecture, we'll use the lifting method to motivate the need for the above concepts.

Consider the following function that compiles even with pre-C++11 compiler.
```cpp
int& double_increment(int& x) 
{int y = x++; return ++x;}
```
It first postfix-increments `x` and stores the result into `y`, which is of type `int`.
And then prefix-increment `x`, hence double-incrementing, and return as an lvalue reference.

Of course, this function actually applies more generally to other types such as `char, double, Complex`, etc.
We can generalize this by using templates as follows:
```cpp
template <class T>
T& double_increment(T& x)
{T y = x++; return ++x;}
```

The problem now is that `T` is _way_ too general - 
what if `x++` or `++x` doesn't even compile for some types?
What if `x++` returns something so crazy that it cannot even be converted to type `T`?
The last step is to _constrain_ the type `T` such that both expressions are valid,
`x++` returns something that can be converted to `T`,
and `++x` returns something of type `T&`.
This is precisely our `Incrementable` concept!
Using concepts, we can fully generalize the function as such:

```cpp
template <Incrementable T>
T& double_increment(T& x)
{T y = x++; return ++x;}
```

The following is a test code that uses `Incrementable` concept:
```cpp
struct incrementable
{
    incrementable& operator++() {return *this;}; 
    incrementable operator++(int) {return *this;} 
    int x = 0;
};

struct not_incrementable
{
    int& operator++() {return ++x;};    // prefix operator++; note: int& not same as not_incrementable&
    not_incrementable operator++(int) 
    {return *this;} 
    int x = 0;
};

template <Incrementable T>
T& double_increment(T& x)
{T y = x++; return ++x;}

int main()
{
    // Sanity-check
    int x = 2;
    assert(double_increment(x) == 4);
    assert(x == 4);

    // Test struct incrementable 
    incrementable inc;
    double_increment(inc);

    // compiler error if the following uncommented
    //not_incrementable ninc;
    //double_increment(ninc);
    
    std::cerr << "PASSED\n";

    return 0;
}
```

Note that the struct `incrementable` satisfies the concept `Incrementable`.
The struct `not_incrementable` satisfies all of the constraints of `Incrementable` except that
the return type of prefix `operator++` is `int&`, which is _not_ the same as `not_incrementable&`.
Hence, you will get a compiler error once you pass it to `double_increment`!

The test code is located in `src` directory - feel free to play around with it!
Try coming up with example functions like `double_increment` for the other concepts (`Decrementable`, `Crementable`),
and write test code to verify your implementation.

#### How do I go about defining concepts for this assignment?

The previous sections covered how one can write _a_ concept.
This assignment specifically requires you to use iterator-related concepts.
This section is for those who are either forced to write concepts on their own (no gcc-10),
or interested in getting practice with writing concepts.

From the lifting method described [above](#how-do-we-use-these-concepts), 
we have to first understand how the generic types should be constrained so we must look at the documentation page for
`std::find, std::find_if, std::sort`.
You will see under `Parameters`, a tiny section called `Type Requirements`, which explains the properties
assumed for each of the template parameters.

For example, `std::find` requires that the template parameter `InputIt` must meet the requirements of `LegacyInputIterator`.
If you check out the page for [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator),
you will see a list of requirements that look eerily similar in format 
as the one for `Incrementable` described in this [section](#user-defined-concepts).
Note that a lot of these concepts build on top of other lower-level concepts - 
you just have to recursively go down.
Feel free to approximate these concepts, but **make sure you are at least constraining the types enough such that
all of the things assumed by the functions `std::find, std::find_if, std::sort` are valid**.
For example, `std::find` will do something like `*first == val`.
So, you must at least constrain your input iterator concept (whatever you end up naming it) such that this expression is valid.

Here is a complete list of links that may be helpful including those referenced in this README :
- [Iterator Library](https://en.cppreference.com/w/cpp/iterator)
    - [iterator_traits](https://en.cppreference.com/w/cpp/iterator/iterator_traits)
- [concepts header](https://en.cppreference.com/w/cpp/header/concepts)
- [concepts documentation](https://en.cppreference.com/w/cpp/experimental/constraints)
- [type_traits header](https://en.cppreference.com/w/cpp/header/type_traits)

----

Hopefully, this was helpful in getting you started with the assignment and understanding what concepts is all about! :)
