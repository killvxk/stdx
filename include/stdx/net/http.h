#pragma once
#include <stdx/env.h>
#include <unordered_map>
#include <string>
#include <memory>

namespace stdx
{
	template<typename _String>
	using http_header = std::unordered_map<_String, _String>;

	enum class http_method
	{
		get,		//GET
		post,		//POST
		put,		//PUT
		del,		//DELETE
		options,	//OPTIONS
		head,		//HEAD
		trace,		//TRACE
		connect,	//CONNECT
		patch		//PATCH
	};
	
	enum class http_version
	{
		http_1_0, //http 1.0
		http_1_1  //http 1.1
	};
	
	template<typename _String>
	class _HttpRequest
	{
		using char_t = _String::value_type;
	public:
		_HttpRequest<_String>()
			:m_method(http_method::get)
			,m_url((char_t*)"/")
			,m_version(http_version::http_1_1)
			,m_header()
			,m_body()
		{}

		_HttpRequest<_String>(const http_method &method)
			:m_method(method)
			,m_url((char_t*)"/")
			,m_version(http_version::http_1_1)
			,m_header()
			,m_body()
		{}

		_HttpRequest<_String>(const http_method &method,const _String &url)
			: m_method(method)
			, m_url(url)
			, m_version(http_version::http_1_1)
			, m_header()
			, m_body()
		{}

		_HttpRequest<_String>(const http_method &method, const _String &url,const http_version &version)
			: m_method(method)
			, m_url(url)
			, m_version(version)
			, m_header()
			, m_body()
		{}

		_HttpRequest<_String>(const _String &url)
			: m_method(http_method::get)
			, m_url(url)
			, m_version(http_version::http_1_1)
			, m_header()
			, m_body()
		{}
		
		_HttpRequest<_String>(const _String &url,const http_version &version)
			: m_method(http_method::get)
			, m_url(url)
			, m_version(version)
			, m_header()
			, m_body()
		{}

		~_HttpRequest<_String>()=default;

		_String &operator[](const _String &key)
		{
			return m_header[key];
		}

		void add_head(const _String &key, const _String &value)
		{
			auto iterator = m_header.find(key);
			if (iterator == std::end(m_header))
			{
				m_header.emplace(key, value);
			}
		}

		void del_head(const _String &key)
		{
			auto iterator = m_header.find(key);
			if (iterator != std::end(m_header))
			{
				m_header.erase(iterator);
			}
		}

		void body(const _String &body)
		{
			m_body = body;
		}

		const _String &body() const
		{
			return m_body;
		}

		void method(const http_method &method)
		{
			m_method = method;
		}

		const http_method &method() const
		{
			return m_method;
		}

		void url(const _String &val)
		{
			m_url = val;
		}

		const _String &url() const
		{
			return m_url;
		}

		void version(const http_version &ver)
		{
			m_version = ver;
		}
			
		const http_version &version() const
		{
			return m_version;
		}

		_String to_string() const
		{
			_String str;
			str.append(build_method());
			str.append((char_t*)" ");
			str.append(url);
			str.append((char_t*)" ");
			str.append(build_version());
			str.append((char_t*)CRLF);
			for (auto iterator = std::begin(m_header),end = std::end(m_header);iterator != end;++iterator)
			{
				str.append(iterator->first);
				str.append((char_t*)":");
				str.append(iterator->second);
				str.append((char_t*)CRLF);
			}
			str.append((char_t*)CRLF);
			if (!m_body.empty())
			{
				str.append(m_body);
			}
			return str;
		}

		delete_copy(_HttpRequest<_String>);
	private:
		http_method m_method;
		_String m_url;
		http_version m_version;
		http_header<_String> m_header;
		_String m_body;

		_String build_method(const http_method &method)
		{
			switch (method)
			{
			case http_method::get:
				return _String((char_t*)"GET");
			case http_method::post:
				return _String((char_t*)"POST");
			case http_method::put:
				return _String((char_t*)"PUT");
			case http_method::del:
				return _String((char_t*)"DELETE");
			case http_method::patch:
				return _String((char_t*)"PATCH");
			case http_method::head:
				return _String((char_t*)"HEAD");
			case http_method::options:
				return _String((char_t*)"OPTIONS");
			case http_method::connect:
				return _String((char_t*)"CONNECT");
			case http_method::trace:
				return _String((char_t*)"TRACE");
			}
		}

		_String build_version(const http_version &ver)
		{
			switch (ver)
			{
			case http_version::http_1_0:
				return _String((char_t*)"HTTP/1.0");
			case http_version::http_1_1:
				return _String((char_t*)"HTTP/1.1");
			default:
				break;
			}
		}
	};

	template<typename _String>
	class http_request
	{
		using impl_t = std::shared_ptr<_HttpRequest<_String>>;
	public:
		http_request<_String>()
			:m_impl(std::make_shared<_HttpRequest<_String>>())
		{}

		http_request<_String>(const http_method &method)
			:m_impl(std::make_shared<_HttpRequest<_String>>(method))
		{}

		http_request<_String>(const http_method &method, const _String &url)
			:m_impl(std::make_shared<_HttpRequest<_String>>(method,url))
		{}

		http_request<_String>(const http_method &method, const _String &url, const http_version &version)
			:m_impl(std::make_shared<_HttpRequest<_String>>(method,url,version))
		{}

		http_request<_String>(const _String &url)
			:m_impl(std::make_shared<_HttpRequest<_String>>(url))
		{}

		http_request<_String>(const _String &url, const http_version &version)
			:m_impl(std::make_shared<_HttpRequest<_String>>(url,version))
		{}

		http_request<_String>(const http_request<_String> &other)
			:m_impl(other.m_impl)
		{}

		http_request<_String>(http_request<_String> &&other)
			: m_impl(std::move(other.m_impl))
		{}

		~http_request() = default;

		_String &operator[](const _String &key)
		{
			return m_impl->operator[](key);
		}

		void add_head(const _String &key, const _String &value)
		{
			return m_impl->add_head(key, value);
		}

		void del_head(const _String &key)
		{
			return m_impl->del_head(key);
		}

		void body(const _String &body)
		{
			return m_impl->body(body);
		}

		const _String &body() const
		{
			return m_impl->body();
		}

		void method(const http_method &method)
		{
			return m_impl->method(method);
		}

		const http_method &method() const
		{
			return m_impl->method();
		}

		void url(const _String &val)
		{
			return m_impl->url(val);
		}

		const _String &url() const
		{
			return m_impl->url();
		}

		void version(const http_version &ver)
		{
			return m_impl->version(ver);
		}

		const http_version &version() const
		{
			return m_impl->version();
		}

		_String to_string() const
		{
			return m_impl->to_string();
		}

		bool operator==(const http_request &other)
		{
			return m_impl == other.m_impl;
		}
	private:
		impl_t m_impl;
	};
}