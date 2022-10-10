#include <iostream>
#include <chrono>
#include <tuple>
#include <vector>
#include <string>
#include <iomanip>

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



typedef unsigned long long _loop_t;
enum base_using { yes, no };
template <base_using T>
using base_using_t = integral_constant<base_using, T>;

template<typename T, operation O, const _loop_t R, enable_if_t<0 == R, bool> = 0>
__forceinline void exec_repeated(volatile T& a, volatile T& b, volatile T& res, const operation_t<O>& op, const base_using_t<no>)
{ exec_operation(a, b, res, op); }
template<typename T, operation O, const _loop_t R, enable_if_t<0 == R, bool> = 0>
__forceinline void exec_repeated(volatile T& a, volatile T& b, volatile T& res, const operation_t<O>& op, const base_using_t<yes>)
{
	exec_operation(a, b, res, op);
	exec_operation(a, b, res, no_operation_constant);
}
template<typename T, operation O, const _loop_t R, base_using B, enable_if_t<0 < R, bool > = 0>
__forceinline void exec_repeated(volatile T& a, volatile T& b, volatile T& res, const operation_t<O>& op, const base_using_t<B>& base)
{
	exec_repeated<T, O, 0>(a, b, res, op, base);
	exec_repeated<T, O, R-1, B>(a, b, res, op, base);
}

template<typename T, operation O, const _loop_t R, base_using B>
double measure_time(volatile T& a, volatile T& b, volatile T& res, const operation_t<O>& op, const base_using_t<B>& base)
{
	chrono::high_resolution_clock::time_point begin{}, end{};

	begin = chrono::high_resolution_clock::now();
	exec_repeated<T, O, R>(a, b, res, op, base);
	end = chrono::high_resolution_clock::now();

	return chrono::duration_cast<chrono::nanoseconds>(end - begin).count();
}


template <typename T, operation O, const _loop_t R>
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
	return 2*ret_not_based - ret_based;
}


template<typename T, const _loop_t R, const _loop_t C>
tuple<string, double> run_test_for(const operation O)
{
	switch (O)
	{
	case addition:
		return {"+", run_test<T, addition, R>(C)};
	case subtraction:
		return {"-", run_test<T, subtraction, R>(C)};
	case multiplication:
		return {"*", run_test<T, multiplication, R>(C)};
	case division:
		return {"/", run_test<T, division, R>(C)};
	default:
		return {"", 0};
	}
}

template<const _loop_t R, const _loop_t C>
tuple<string, string, double> run_test_for(const int type_id, const operation O)
{
	switch (type_id)
	{
	case 0:
		return tuple_cat(make_tuple("int"), run_test_for<int, R, C>(O));
	case 1:
		return tuple_cat(make_tuple("long"), run_test_for<long, R, C>(O));
	case 2:
		return tuple_cat(make_tuple("llong"), run_test_for<long long, R, C>(O));
	case 3:
		return tuple_cat(make_tuple("char"), run_test_for<char, R, C>(O));
	case 4:
		return tuple_cat(make_tuple("double"), run_test_for<double, R, C>(O));
	default:
		return tuple_cat(make_tuple(""), run_test_for<int, R, C>(O));
	}
}

void print_result(const double max, const tuple<string, string, double>& result)
{
	constexpr int bar_len = 50;

	cout << setw(2) << std::left << std::get<1>(result)
		<< setw(10) << std::left << std::get<0>(result)
		<< setw(16) << std::left << std::get<2>(result)
		<< setw(bar_len + 1) << std::left << string(round(std::get<2>(result) * bar_len / max), 'X')
		<< round(std::get<2>(result) * 100 / max) << "%\n";
}

int main()
{
	cout.precision(6);
	double max = 0;
	vector<tuple<string, string, double>> results(0);
	const int types_num = 5;
	for (int type = 0; type < types_num; type++)
		for (int op = 0; op < 4; op++) {
			results.push_back(run_test_for<50, (_loop_t)1e5>(type, (operation)op));
			double buff = std::get<2>(*(results.end() - 1));
			if (buff > max) max = buff;
		}

	cout << max << "\n";
	for (int i = 0; i < 4 * types_num; i++) {
		if (i % 4 == 0 && i != 0) cout << "\n";
		print_result(max, results[i]);
	}
	return 0;
}