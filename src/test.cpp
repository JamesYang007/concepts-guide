#include <iostream>
#include <cassert>
#include <concepts>
#include <type_traits>

//////////////////////////////////////////
// Concept Definitions
//////////////////////////////////////////

template <class T>
concept Incrementable = 
    requires(T x) {
        { x++ } -> std::convertible_to<T>;
        { ++x } -> std::same_as< std::add_lvalue_reference_t<
                                 std::remove_reference_t<T> > >;
    };

//////////////////////////////////////////
// Dummy Class Definitions
//////////////////////////////////////////

struct not_incrementable
{
    int& operator++() {return ++x;};    // prefix operator++; int& not same as not_incrementable&
    not_incrementable operator++(int)   // note: not_incrementable is (trivially) convertible to not_incrementable
    {return *this;} 
    int x = 0;
};

//////////////////////////////////////////
// Dummy Function Definitions
//////////////////////////////////////////

// double_increment function should only work for Incrementable types
template <Incrementable T>
T& double_incrementable(T& x)
{T y = x++; return ++x;}

int main()
{
    int x = 2;
    assert(double_incrementable(x) == 4);
    assert(x == 4);

    // compiler error if the following uncommented
    //not_incrementable y;
    //double_incrementable(y);
    
    std::cerr << "PASSED\n";

    return 0;
}
