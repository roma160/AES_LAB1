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
__forceinline void exec_operation(const T a, const T b, volatile T& res, operation_t<addition>)
{ res = a + b; }
template<typename T>
__forceinline void exec_operation(const T a, const T b, volatile T& res, operation_t<subtraction>)
{ res = a - b; }
template<typename T>
__forceinline void exec_operation(const T a, const T b, volatile T& res, operation_t<multiplication>)
{ res = a * b; }
template<typename T>
__forceinline void exec_operation(const T a, const T b, volatile T& res, operation_t<division>)
{ res = a / b; }
template<typename T>
__forceinline void exec_operation(const T a, const T b, volatile T& res, operation_t<no_operation>)
{ res = a; }

template<typename T, const unsigned int end, operation O, const unsigned int start = 0>
__forceinline void exec_operation(volatile T a, volatile T b, volatile T& res, const operation_t<O>& op)
{
	exec_operation(a, b, res, op);
	if constexpr (start + 1 < end)
		exec_operation<T, end, O, start + 1>(a, b, res, op);
}

typedef unsigned long long _loop_t;
template <operation O, typename T, const unsigned int repeat>
double exec(const _loop_t count = 100)
{
	const operation_t<O> op_t{};
	const auto begin = chrono::high_resolution_clock::now();

	T r = 0, a = 1, b = 1;
	for (_loop_t i = 0; i < count; i++) {
		// https://stackoverflow.com/questions/37980791/conditional-function-invocation-using-template
		exec_operation<T, repeat>(a, b, r, op_t);
	}

	const auto end = chrono::high_resolution_clock::now();
	return (double)chrono::duration_cast<chrono::milliseconds>(end - begin).count();
}


template <operation O, typename T, const unsigned int repeat = 20>
double check(const _loop_t count = 100)
{
	double micros_op = exec<O, T, repeat>(count);
	double micros_no = exec<no_operation, T, repeat>(count);
	return count * 1e6 * repeat / (micros_op);
}

int main()
{
	cout.precision(4);
	cout << check<addition, int>(1e8);
	return 0;
}