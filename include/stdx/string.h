#pragma once
#include <stdx/env.h>
#include <string>
namespace stdx
{
	template<typename _String,typename _Container>
	inline void _SpitStr(_String &str,const _String &chars, _Container &container)
	{
		if (chars.empty())
		{
			throw std::invalid_argument("argument chars can not be empty");
		}
		size_t pos = str.find(chars);
		if (pos == std::string::npos)
		{
			container.push_back(str);
		}
		else
		{
			if (pos != 0)
			{
				container.push_back(str.substr(0,pos));
			}
			if (pos != (str.size()-1))
			{
				_SpitStr(str.substr(pos + chars.size(),str.size()-1), chars, container);
			}
		}
	}

	template<typename _Container,typename _String=std::string>
	inline void spit_string(_String &str, const _String &chars,_Container &container)
	{
		return _SpitStr(str, chars, container);
	}
}