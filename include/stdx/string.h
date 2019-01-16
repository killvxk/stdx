#pragma once
#include <string>

namespace stdx
{
	class _String
	{
	public:
		_String(const char *str)
			:m_data(str)
		{}
		_String(const char *str,const size_t &size)
			:m_data(str,size)
		{}

		~_String() = default;

	private:
		std::string m_data;
	};
}