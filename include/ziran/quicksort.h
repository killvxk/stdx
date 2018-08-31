#pragma once
#include <vector>

namespace ziran
{
	//快速排序方法(从大到小)
	template<typename T, typename TContainer = std::vector<T>>
	void quicksort_bigger(TContainer &container, size_t begin, size_t end)
	{
		if (end == 0)
		{
			return;
		}
		end = end - 1;
		if (begin > end)
		{
			return;
		}
		auto basic = container[begin];
		auto begin_ps = begin;
		auto end_ps = end;
		while (begin != end)
		{
			while (container[end] <= basic && begin < end)
			{
				end--;
			}
			while (container[begin] >= basic && begin < end)
			{
				begin++;
			}
			if (begin<end)
			{
				auto temp = container[begin];
				container[begin] = container[end];
				container[end] = temp;
			}
		}
		container[begin_ps] = container[begin];
		container[begin] = basic;
		quicksort_bigger<T, TContainer>(container, begin_ps, begin);
		quicksort_bigger<T, TContainer>(container, begin + 1, end_ps+1);
	}
	//快速排序方法(从小到大)
	template<typename T, typename TContainer = std::vector<T>>
	void quicksort_smaller(TContainer &container, size_t begin, size_t end)
	{
		if (end == 0)
		{
			return;
		}
		end = end - 1;
		if (begin > end)
		{
			return;
		}
		auto basic = container[begin];
		auto begin_ps = begin;
		auto end_ps = end;
		while (begin != end)
		{
			while (container[end] >= basic && begin < end)
			{
				end--;
			}
			while (container[begin] <= basic && begin < end)
			{
				begin++;
			}
			if (begin<end)
			{
				auto temp = container[begin];
				container[begin] = container[end];
				container[end] = temp;
			}
		}
		container[begin_ps] = container[begin];
		container[begin] = basic;
		quicksort_smaller<T, TContainer>(container, begin_ps, begin);
		quicksort_smaller<T, TContainer>(container, begin + 1, end_ps+1);
	}
}