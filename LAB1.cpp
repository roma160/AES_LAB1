#include <iostream>
#include <chrono>

using namespace std;

enum operation
{
	addition,
	subtraction,
	multiplication,
	division,
	no_operation
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
template<typename T>
__forceinline T exec_operation(T a, T b, operation_t<no_operation>)
{ return a; }


template <operation O, typename T>
double exec(const unsigned int count = 100)
{
	const auto begin = chrono::high_resolution_clock::now();

	volatile T res = 0, b = 1;
	for (unsigned int i = 0; i < count; i++) {
		// https://stackoverflow.com/questions/37980791/conditional-function-invocation-using-template
		res = exec_operation<T>(res, b, operation_t<O>());
	}

	const auto end = chrono::high_resolution_clock::now();
	return (double)chrono::duration_cast<chrono::milliseconds>(end - begin).count();
}

int main()
{
	cout.precision(4);
	double operations = 1e7;
	double micros_op = exec<addition, int>(operations);
	double micros_no = exec<no_operation, int>(operations);
	cout << operations * 1e6 / (micros_op - micros_no);
    return 0;
}