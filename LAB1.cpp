#include <iostream>
#include <chrono>
#include <tuple>

using namespace std;

enum operation
{
	addition,
	subtraction,
	multiplication,
	division,
	no_operation
};
template <operation T>
using operation_t = integral_constant<operation, T>;
constexpr operation_t<no_operation> no_operation_constant{};

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
{ res = a; res = b; }



enum base_using { yes, no };
template <base_using T>
using base_using_t = integral_constant<base_using, T>;

template<typename T, operation O, const size_t R, enable_if_t<0 == R, bool> = 0>
__forceinline void exec_repeated(volatile T& a, volatile T& b, volatile T& res, const operation_t<O>& op, const base_using_t<no>)
{ exec_operation(a, b, res, op); }
template<typename T, operation O, const size_t R, enable_if_t<0 == R, bool> = 0>
__forceinline void exec_repeated(volatile T& a, volatile T& b, volatile T& res, const operation_t<O>& op, const base_using_t<yes>)
{
	exec_operation(a, b, res, op);
	exec_operation(a, b, res, no_operation_constant);
}
template<typename T, operation O, const size_t R, base_using B, enable_if_t<0 < R, bool > = 0>
__forceinline void exec_repeated(volatile T& a, volatile T& b, volatile T& res, const operation_t<O>& op, const base_using_t<B>& base)
{
	exec_repeated<T, O, 0>(a, b, res, op, base);
	exec_repeated<T, O, R-1, B>(a, b, res, op, base);
}

template<typename T, operation O, const size_t R, base_using B>
double measure_time(volatile T& a, volatile T& b, volatile T& res, const operation_t<O>& op, const base_using_t<B>& base)
{
	chrono::high_resolution_clock::time_point begin{}, end{};

	begin = chrono::high_resolution_clock::now();
	exec_repeated<T, O, R>(a, b, res, op, base);
	end = chrono::high_resolution_clock::now();

	return chrono::duration_cast<chrono::nanoseconds>(end - begin).count();
}


typedef unsigned long long _loop_t;
template <typename T, operation O, const unsigned int R>
double run_test(const _loop_t count = 100)
{
	constexpr base_using_t<yes> op_based{};
	constexpr base_using_t<no> op_not_based{};

	const operation_t<O> op{};
	double ret_based = 0, ret_not_based = 0;
	T res = 0, a = 1, b = 1;
	for (_loop_t i = 0; i < count; i++) {
		ret_based += measure_time<T, O, R>(a, b, res, op, op_based);
		ret_not_based += measure_time<T, O, R>(a, b, res, op, op_not_based);
	}
	return ret_based - ret_not_based;
}


int main()
{
	cout.precision(4);

	const long num = 1e5;
	double basing_yes = run_test<int, addition, 50>(num);
	cout << basing_yes << "\n";
	return 0;
}