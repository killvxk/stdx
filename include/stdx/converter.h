#pragma once
#include <string>

namespace stdx
{
	struct converter
	{
		template<typename T>
		static std::string to_string(const T &i)
		{
			return std::to_string(i);
		}

		static int to_int(const std::string &str)
		{
			return atoi(str.c_str());
		}

		static int to_int(const double &d)
		{
			return static_cast<int>(d);
		}

		static unsigned int to_uint(const std::string &str)
		{
			return strtoul(str.c_str(),0,0);
		}

		static unsigned long to_ulong(const std::string &str)
		{
			return strtoul(str.c_str(), 0, 0);
		}

		static long to_long(const std::string &str)
		{
			return strtol(str.c_str(), 0, 0);
		}

		static double to_double(const std::string &str)
		{
			return strtod(str.c_str(), 0);
		}
	};
}