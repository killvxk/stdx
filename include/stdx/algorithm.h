#pragma once
#include <vector>

namespace stdx
{
	struct sort_way
	{
		enum
		{
			bigger = 0,
			smaller = 1
		};
	};
	//快速排序方法(从大到小)
	template<typename _T, typename _TContainer = std::vector<_T>>
	void quicksort_bigger(_TContainer &container, size_t begin, size_t end)
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
		quicksort_bigger<_T, _TContainer>(container, begin_ps, begin);
		quicksort_bigger<_T, _TContainer>(container, begin + 1, end_ps + 1);
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
		quicksort_smaller<T, TContainer>(container, begin + 1, end_ps + 1);
	}

	//将字符串分割成两个部分
	template<typename _TString = std::string>
	std::pair<_TString, _TString> split_to_double(const _TString &str, const _TString &pattern)
	{
		//如果是空则报错
		if (pattern.empty())
		{
			throw std::invalid_argument("参数 pattern 不能为空字符串,无法根据pattern分割str");
		}
		//定义一个pair来储存数据
		auto pair = std::make_pair<std::string, std::string>("", "");
		//使用STL分割
		//先找到要被分割字符串的位置
		size_t pos = str.find(pattern);
		//如果找不到
		if (pos == str.npos)
		{
			//判断str是否为空
			if (!str.empty())
			{
				//如果不是说明没有这个字符串
				//把全部设置到first项
				pair.first = str;
			}
		}
		//找到了
		else
		{
			//设置first项
			pair.first = str.substr(0, pos);
			//设置second项,注意偏移量
			pair.second = str.substr(pos + pattern.size(), str.size());
		}
		return pair;
	}

	//将字符串分割成几个部分
	template<typename _TString = std::string>
	std::vector<_TString> split_string(const _TString &str, const _TString &pattern)
	{
		//创建vector储存结果
		std::vector<_TString> result;
		//先分割成一半
		auto pair = split_to_half<_TString>(str, pattern);
		//将first push_back
		result.push_back(pair.first);
		//查看是否需要继续分割
		while (!pair.second.empty())
		{
			//继续分割一半
			pair = split_to_half(pair.second, pattern);
			//将first push_back
			result.push_back(pair.first);
		}
		//返回结果
		return result;
	}
}