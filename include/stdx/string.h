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
		if (pos == _String::npos)
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

	template<typename _String>
	inline void replace_string(_String &str,const _String &target,const _String &val)
	{
		size_t pos = str.find(target);
		if (pos == _String::npos)
		{
			return;
		}
		else
		{
			str.replace(pos,target.size(), val);
			return replace_string(str, target, val);
		}
	}
#ifdef WIN32
#define _ThrowWinError auto _ERROR_CODE = GetLastError(); \
						LPVOID _MSG;\
						if(_ERROR_CODE != ERROR_IO_PENDING) \
						{ \
							if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,_ERROR_CODE,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &_MSG,0,NULL))\
							{ \
								throw std::runtime_error((char*)_MSG);\
							}else \
							{ \
								std::string _ERROR_MSG("windows system error:");\
								_ERROR_MSG.append(std::to_string(_ERROR_CODE));\
								throw std::system_error(std::error_code(_ERROR_CODE,std::system_category()),_ERROR_MSG.c_str()); \
							} \
						}\

	struct code_page
	{
		enum 
		{
			ansi = CP_ACP,
			ansi_current_thread = CP_THREAD_ACP,
			utf8 = CP_UTF8
		};
	};

	//template<typename _FormString, typename _WString>
	//inline void encode(uint code_page)
	//{

	//}
	template<typename _String = std::string,typename _WString>
	inline _String unicode_to_utf8(const _WString &src)
	{
		DWORD size = ::WideCharToMultiByte(stdx::code_page::utf8, NULL, src.c_str(), -1, NULL, 0, NULL, FALSE);
		char *buf = (char*)calloc(size, sizeof(char));
		if (!(::WideCharToMultiByte(stdx::code_page::utf8,NULL,src.c_str(),-1,buf,size,NULL,FALSE)))
		{
			_ThrowWinError
		}
		_String des = buf;
		free(buf);
		return des;
	}

	template<typename _String = std::string, typename _WString>
	inline _String unicode_to_ansi(const _WString &src)
	{
		DWORD size = ::WideCharToMultiByte(stdx::code_page::ansi, NULL, src.c_str(), -1, NULL, 0, NULL, FALSE);
		char *buf = (char*)calloc(size, sizeof(char));
		if (!(::WideCharToMultiByte(stdx::code_page::ansi, NULL, src.c_str(), -1, buf, size, NULL, FALSE)))
		{
			_ThrowWinError
		}
		_String des = buf;
		free(buf);
		return des;
	}

	template<typename _WString = std::wstring,typename _String>
	inline _WString utf8_to_unicode(const _String &src)
	{
		DWORD size = ::MultiByteToWideChar(stdx::code_page::utf8, NULL, src.c_str(), -1, NULL, 0);
		wchar_t *buf = (wchar_t*)calloc(size, sizeof(wchar_t));
		if (!(::MultiByteToWideChar(stdx::code_page::utf8, NULL, src.c_str(), -1, buf, size)))
		{
			_ThrowWinError
		}
		_WString des = buf;
		free(buf);
		return des;
	}

	template<typename _String = std::string>
	inline _String utf8_to_ansi(const _String &src)
	{
		std::wstring temp = stdx::utf8_to_unicode(src);
		return stdx::unicode_to_ansi(temp);
	}

	template<typename _WString = std::wstring, typename _String>
	inline _WString ansi_to_unicode(const _String &src)
	{
		DWORD size = ::MultiByteToWideChar(stdx::code_page::ansi, NULL, src.c_str(), -1, NULL, 0);
		wchar_t *buf = (wchar_t*)calloc(size, sizeof(wchar_t));
		if (!(::MultiByteToWideChar(stdx::code_page::ansi, NULL, src.c_str(), -1, buf, size)))
		{
			_ThrowWinError
		}
		_WString des = buf;
		free(buf);
		return des;
	}

	template<typename _String = std::string>
	inline _String ansi_to_utf8(const _String &src)
	{
		std::wstring temp = stdx::ansi_to_unicode(src);
		return stdx::unicode_to_utf8(temp);
	}

	
#undef _ThrowWinError
#endif
}