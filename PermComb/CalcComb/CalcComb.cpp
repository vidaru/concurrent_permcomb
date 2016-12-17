#include <chrono>
#include <iostream>
#include <iomanip>
#include <intrin.h>
#include <boost/multiprecision/cpp_int.hpp>
#include "../permcomb/concurrent_comb.h"

void test_find_comb(uint32_t fullset, uint32_t subset);
void benchmark_comb();

class timer
{
public:
	timer() = default;
	void start_timing(const std::string& text_)
	{
		text = text_;
		begin = std::chrono::high_resolution_clock::now();
	}
	void stop_timing()
	{
		auto end = std::chrono::high_resolution_clock::now();
		auto dur = end - begin;
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
		std::cout << std::setw(16) << text << ":" << std::setw(5) << ms << "ms" << std::endl;
	}

private:
	std::string text;
	std::chrono::high_resolution_clock::time_point begin;
};

template<typename T>
bool compare_vec(std::vector<T>& results1, std::vector<T>& results2)
{
	if (results1.size() != results2.size())
		return false;

	for (size_t i = 0; i<results1.size(); ++i)
	{
		if (results1[i] != results2[i])
			return false;
	}
	return true;
}

template<typename T>
void display(std::vector<T>& results)
{
	for (size_t i = 0; i<results.size(); ++i)
	{
		std::cout << results[i] << ", ";
	}
	std::cout << std::endl;
}

template<typename int_type>
bool how_to_use_thread_index_comb(int_type thread_cnt, uint32_t fullset_size, uint32_t subset_size)
{
	std::vector<uint32_t> fullset(fullset_size);
	std::iota(fullset.begin(), fullset.end(), 0);

	std::vector<std::vector< std::vector<uint32_t> > > vecvecvec((size_t)thread_cnt);

	concurrent_comb::compute_all_comb(thread_cnt, subset_size, fullset.cbegin(), fullset.cend(),
		[&vecvecvec](const int_type thread_index, 
			uint32_t fullset_cnt, uint32_t subset_cnt, 
			auto begin, auto end) -> bool
		{
			vecvecvec[(size_t)thread_index].push_back(std::vector<uint32_t>(begin, end));
			return true;
		});

	std::vector<uint32_t> subset(subset_size);
	std::iota(subset.begin(), subset.end(), 0);
	std::vector< std::vector<uint32_t> > vecvec;
	do
	{
		vecvec.push_back(std::vector<uint32_t>(subset.begin(), subset.end()));
	} while (stdcomb::next_combination(fullset.begin(), fullset.end(), subset.begin(), subset.end()));

	// compare results
	size_t cnt = 0;
	for (size_t i = 0; i < vecvecvec.size(); ++i)
	{
		for (size_t j = 0; j < vecvecvec[i].size(); ++j, ++cnt)
		{
			if (!compare_vec(vecvec[cnt], vecvecvec[i][j]))
			{
				std::cout << "Perm at " << cnt << " is not the same!" << std::endl;

				display(vecvec[cnt]);
				display(vecvecvec[i][j]);

				return false;
			}
		}
	}
	std::cout << "how_to_use_thread_index_comb done! thread_cnt: " << thread_cnt 
		<< ", fullset_size: " << fullset_size 
		<< ", subset_size: "  << subset_size 
		<< std::endl;

	return true;
}

// return false to stop processing
template<typename int_type, typename bidirectional_iterator>
struct empty_callback_t
{
	bool operator()(const int_type thread_index, uint32_t fullset,
		uint32_t subset, bidirectional_iterator begin, bidirectional_iterator end)
	{
		return true;
	}
};

//typedef boost::multiprecision::cpp_int int_type;
typedef boost::multiprecision::int256_t int_type;

int main(int argc, char* argv[])
{
	int_type thread_cnt = 3;
	how_to_use_thread_index_comb(thread_cnt, 10, 5);

	//benchmark_comb();

	//test_find_comb(5, 2);

	return 0;
}

void benchmark_comb()
{
	std::vector<int> fullset_vec(24);
	std::iota(fullset_vec.begin(), fullset_vec.end(), 0);

	int_type thread_cnt = 1;
	uint32_t subset = 12;

	typedef empty_callback_t<int_type, decltype(fullset_vec.cbegin())> callback_t;

	timer stopwatch;
	stopwatch.start_timing("1 thread(s)");
	thread_cnt = 1;
	concurrent_comb::compute_all_comb(thread_cnt, subset, fullset_vec.cbegin(), fullset_vec.cend(), callback_t());
	stopwatch.stop_timing();

	stopwatch.start_timing("2 thread(s)");
	thread_cnt = 2;
	concurrent_comb::compute_all_comb(thread_cnt, subset, fullset_vec.cbegin(), fullset_vec.cend(), callback_t());
	stopwatch.stop_timing();

	stopwatch.start_timing("3 thread(s)");
	thread_cnt = 3;
	concurrent_comb::compute_all_comb(thread_cnt, subset, fullset_vec.cbegin(), fullset_vec.cend(), callback_t());
	stopwatch.stop_timing();

	stopwatch.start_timing("4 thread(s)");
	thread_cnt = 4;
	concurrent_comb::compute_all_comb(thread_cnt, subset, fullset_vec.cbegin(), fullset_vec.cend(), callback_t());
	stopwatch.stop_timing();
}

void test_find_comb(uint32_t fullset, uint32_t subset)
{
	std::vector<uint32_t> fullset_vec(fullset);
	std::vector<uint32_t> results1(subset);
	std::vector<uint32_t> results2(subset);

	std::iota(fullset_vec.begin(), fullset_vec.end(), 0);
	std::iota(results1.begin(), results1.end(), 0);
	std::iota(results2.begin(), results2.end(), 0);

	uint32_t nTotal = 0;
	if (!concurrent_comb::find_total_comb(fullset, subset, nTotal))
	{
		std::cerr << "find_total_comb() returns false" << std::endl;
		return;
	}

	for(uint32_t j=0; j<nTotal; ++j )
	{
		if(concurrent_comb::find_comb(fullset, subset, j, results1))
		{
			if(compare_vec(results1, results2)==false)
			{
				std::cout << "Perm at " << j << " is not the same!" << std::endl;
				display(results1);
				display(results2);
			}
			else
			{
				display(results1);
			}
		}
		stdcomb::next_combination(fullset_vec.begin(), fullset_vec.end(), results2.begin(), results2.end());
	}
}

