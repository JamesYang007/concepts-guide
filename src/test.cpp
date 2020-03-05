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

struct incrementable
{
    incrementable& operator++() {return *this;}; 
    incrementable operator++(int) {return *this;} 
    int x = 0;
};

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
