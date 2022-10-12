#include <iostream>
#include <chrono>
#include <tuple>
#include <vector>
#include <string>
#include <iomanip>
#include <cmath>

using namespace std;

#if defined(__GNUC__) || defined(__GNUG__)
#define in_void inline void __attribute__((always_inline))
#else
#define in_void __forceinline void
#endif

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
in_void exec_operation(op_arg_t<T> a, op_arg_t<T> b, volatile T& res, operation_t<addition>)
{ res = a + b; }
template<typename T>
in_void exec_operation(op_arg_t<T> a, op_arg_t<T> b, volatile T& res, operation_t<subtraction>)
{ res = a - b; }
template<typename T>
in_void exec_operation(op_arg_t<T> a, op_arg_t<T> b, volatile T& res, operation_t<multiplication>)
{ res = a * b; }
template<typename T>
in_void exec_operation(op_arg_t<T> a, op_arg_t<T> b, volatile T& res, operation_t<division>)
{ res = a / b; }
template<typename T>
in_void exec_operation(op_arg_t<T> a, op_arg_t<T> b, volatile T& res, operation_t<no_operation>)
{
	res = a;
	//res = b;
}



typedef unsigned long long _loop_t;
enum base_using { yes, no };
template <base_using T>
using base_using_t = integral_constant<base_using, T>;

template <typename T>
using rep_arg_t = volatile T&;

template<typename T, operation O, const _loop_t R, enable_if_t<0 == R, bool> = 0>
in_void exec_repeated(rep_arg_t<T> a, rep_arg_t<T> b, volatile T& res, const operation_t<O>& op, const base_using_t<no>)
{
	exec_operation(a, b, res, op);
}
template<typename T, operation O, const _loop_t R, enable_if_t<0 == R, bool> = 0>
in_void exec_repeated(rep_arg_t<T> a, rep_arg_t<T> b, volatile T& res, const operation_t<O>& op, const base_using_t<yes>)
{
	exec_operation(a, b, res, op);
	exec_operation(a, b, res, no_operation_constant);
}
template<typename T, operation O, const _loop_t R, base_using B, enable_if_t<0 < R, bool > = 0>
in_void exec_repeated(rep_arg_t<T> a, rep_arg_t<T> b, volatile T& res, const operation_t<O>& op, const base_using_t<B>& base)
{
	exec_repeated<T, O, 0>(a, b, res, op, base);
	exec_repeated<T, O, R-1, B>(a, b, res, op, base);
}

template<typename T, operation O, const _loop_t R, base_using B>
double measure_time(rep_arg_t<T> a, rep_arg_t<T> b, volatile T& res, const operation_t<O>& op, const base_using_t<B>& base)
{
	chrono::high_resolution_clock::time_point begin{}, end{};

	begin = chrono::high_resolution_clock::now();
	exec_repeated<T, O, R>(a, b, res, op, base);
	end = chrono::high_resolution_clock::now();

	return chrono::duration_cast<chrono::nanoseconds>(end - begin).count();
}


#pragma optimize( "", off )
#pragma GCC push_options
#pragma GCC optimize ("O0")
template <typename T, operation O, const _loop_t R>
double run_test(const _loop_t count = 100)
{
	constexpr base_using_t<yes> op_based{};
	constexpr base_using_t<no> op_not_based{};

	const operation_t<O> op{};
	double ret = 0, buff;
	T res = 0, a = 1, b = 1;
	for (_loop_t i = 0; i < count; i++) {
		buff = 2 * measure_time<T, O, R>(a, b, res, op, op_not_based) -
			measure_time<T, O, R>(a, b, res, op, op_based);
		if(buff < 0)
		{
			i--;
			continue;
		}
		ret += buff;
	}
	return R * count * 1e9 / ret;
}
#pragma GCC pop_options
#pragma optimize( "", on )


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
		return tuple_cat(make_tuple("float"), run_test_for<float, R, C>(O));
	case 5:
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

constexpr int types_num = 6, ops_num = 4;
void take_measurements(vector<vector<tuple<string, string, double>>> &results, double& max, double& avg)
{
	constexpr _loop_t R = 100, C = 1e5;
	constexpr double infty = numeric_limits<double>::infinity();

	max = avg = 0;
	results = vector<vector<tuple<string, string, double>>>(types_num, vector<tuple<string, string, double>>(ops_num));

	tuple<string, string, double> buff_result{};
	double& time = std::get<2>(buff_result);
	for (int type = 0; type < types_num; type++)
		for (int op = 0; op < ops_num; op++) {
			time = 0;
			while (time <= 0 || !(time < infty)) {
				buff_result = run_test_for<R, C>(type, (operation)op);
			}
			results[type][op] = buff_result;

			if (time > max) max = time;
			avg += time;
		}
	avg /= types_num * ops_num;
}

int main()
{
	cout.precision(6);
	double max = 1e9, avg = 1;
	vector<vector<tuple<string, string, double>>> results;

	// Running tests till system is stable
	while (max / avg > 5) {
		take_measurements(results, max, avg);
	}

	cout << max << "\n";
	for (int type = 0; type < types_num; type++) {
		for (int op = 0; op < ops_num; op++)
			print_result(max, results[type][op]);
		cout << "\n";
	}
	return 0;
}
