#pragma once
#include <vector>

namespace stdx
{
	class _String
	{
	public:
		_String();
		~_String() = default;

	private:
		std::vector<char> m_data;
	};
}