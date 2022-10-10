#include <iostream>
#include <chrono>
#include <Windows.h>

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

template <typename T>
using op_arg_t = T;

template<typename T>
__forceinline void exec_operation(op_arg_t<T> a, op_arg_t<T> b, volatile T& res, operation_t<addition>)
{ res = a + b; }
template<typename T>
__forceinline void exec_operation(op_arg_t<T> a, op_arg_t<T> b, volatile T& res, operation_t<subtraction>)
{ res = a - b; }
template<typename T>
__forceinline void exec_operation(op_arg_t<T> a, op_arg_t<T> b, volatile T& res, operation_t<multiplication>)
{ res = a * b; }
template<typename T>
__forceinline void exec_operation(op_arg_t<T> a, op_arg_t<T> b, volatile T& res, operation_t<division>)
{ res = a / b; }
template<typename T>
__forceinline void exec_operation(op_arg_t<T> a, op_arg_t<T> b, volatile T& res, operation_t<no_operation>)
{ res = a; }


template<typename T, operation O, const size_t R, enable_if_t<0 == R, bool> = 0>
__forceinline void exec_operation(volatile T a, volatile T b, volatile T& res, const operation_t<O>& op)
{ exec_operation(a, b, res, op); }

template<typename T, operation O, const size_t R, enable_if_t<0 < R, bool> = 0>
__forceinline void exec_operation(volatile T a, volatile T b, volatile T& res, const operation_t<O>& op)
{
	exec_operation(a, b, res, op);
	exec_operation<T, O, R - 1>(a, b, res, op);
}

enum use_grounding
{ yes, no };
template <use_grounding t>
using ground_t = integral_constant<use_grounding, t>;

template<typename T, operation O, const size_t R>
__forceinline void exec_operation(volatile T a, volatile T b, volatile T& res, const operation_t<O>& op, const ground_t<yes>&)
{
	exec_operation<T, O, R>(a, b, res, op);
	
}
template<typename T, operation O, const size_t R>
__forceinline void exec_operation(volatile T a, volatile T b, volatile T& res, const operation_t<O>& op, const ground_t<yes>&)
{
	exec_operation(a, b, res, op);
	exec_operation<T, O, R - 1>(a, b, res, op);
}

typedef unsigned long long _loop_t;
template <operation O, typename T, const unsigned int R>
double exec(const _loop_t count = 100)
{
	const operation_t<O> op_t{};
	const auto begin = chrono::high_resolution_clock::now();

	T r = 0, a = 1, b = 1;
	for (_loop_t i = 0; i < count; i++) {
		// https://stackoverflow.com/questions/37980791/conditional-function-invocation-using-template
		exec_operation<T, O, R>(a, b, r, op_t);
	}

	const auto end = chrono::high_resolution_clock::now();
	return (double) chrono::duration_cast<chrono::nanoseconds>(end - begin).count();
}


// TODO: fix check function
template <operation O, typename T, const unsigned int repeat = 20>
double check(const _loop_t count = 100)
{
	double t_op1 = 1, t_op2 = 0, t_no2 = 0, t_no1 = 0, delta;
	while ((delta = t_op2 - t_op1 - t_no2) <= 0)
	{
		t_op1 = exec<O, T, repeat>(count);
		t_op2 = exec<O, T, 2 * repeat>(count);
		t_no1 = exec<no_operation, T, 1>(count);
		t_no2 = exec<no_operation, T, repeat>(count);
	}
	return (double)count / delta;
}


int main()
{
	cout.precision(4);

	for (int i = 0; i < 5; i++) {
		double sum = 0;
		const size_t l = 1e2;
		for(int i = 0; i < l; i++)
			sum += exec<addition, int, 100>(10);
		sum /= l;
		cout << sum;

		cout << "\n";

		double _sum = sum;

		sum = 0;
		for (int i = 0; i < l; i++)
			sum += exec_g<addition, int, 100>(10);
		sum /= l;
		cout << sum << "\n";
		cout << sum - _sum <<"\n||||||||||||||||||\n";
	}
	return 0;
}