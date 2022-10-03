#include <iostream>

using namespace std;

enum operation
{
	addition,
	subtraction,
	multiplication,
	division
};

// https://stackoverflow.com/questions/11430015/how-to-guarantee-that-a-function-is-inlined-in-c
template <operation t>
using operation_t = integral_constant<operation, t>;

template<typename T>
__forceinline T exec_operation(T a, T b, operation_t<addition>)
{ return a + b; }
template<typename T>
__forceinline T exec_operation(T a, T b, operation_t<subtraction>)
{ return a - b; }
template<typename T>
__forceinline T exec_operation(T a, T b, operation_t<multiplication>)
{ return a * b; }
template<typename T>
__forceinline T exec_operation(T a, T b, operation_t<division>)
{ return a / b; }

// https://learn.microsoft.com/en-us/cpp/preprocessor/optimize?view=msvc-170
#pragma optimize("", off)
int exec()
{
	// https://stackoverflow.com/questions/37980791/conditional-function-invocation-using-template
	return exec_operation(1, 2, operation_t<multiplication>());
}
#pragma optimize("", on)

int main()
{ 
	cout << exec();
    return 0;
}