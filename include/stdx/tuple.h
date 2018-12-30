#pragma once
#include <stdx/traits/type_list.h>
#include <memory>
namespace stdx
{
	//Tuple模板实现
	template<typename _First,typename ..._More>
	class _Tuple;

	//一个元素特化
	template<typename _First>
	class _Tuple<_First>
	{
	public:
		_Tuple(const _First &first)
			:m_first(first)
		{}

		~_Tuple() = default;

		template<int index>
		_First &get();

		template<>
		_First &get<0>()
		{
			return m_first;
		}

		template<int index>
		void set(const _First &other);

		template<>
		void set<0>(const _First &other)
		{
			m_first = other;
		}
	private:
		_First m_first;
	};
	//两个元素特化
	template<typename _First,typename _Secound>
	class _Tuple<_First, _Secound>
	{
		using tl_t = stdx::type_list<_First, _Secound>;
	public:
		_Tuple(const _First &first,const _Secound &secound)
			:m_first(first)
			,m_secound(secound)
		{}

		~_Tuple() = default;

		template<int index>
		stdx::type_at<index,tl_t> &get();

		template<>
		stdx::type_at<0, tl_t> &get<0>()
		{
			return m_first;
		}

		template<>
		stdx::type_at<1,tl_t> &get<1>()
		{
			return m_secound;
		}

		template<int index>
		void set(const stdx::type_at<index,tl_t> &other);

		template<>
		void set<0>(const stdx::type_at<0, tl_t> &other)
		{
			m_first = other;
		}
		template<>
		void set<1>(const stdx::type_at<1, tl_t> &other)
		{
			m_secound = other;
		}
	private:
		_First m_first;
		_Secound m_secound;
	};

	//多个元素特化
	template<typename _First, typename _Secound,typename ..._More>
	class _Tuple<_First, _Secound,_More...>
	{
		using tl_t = stdx::type_list<_First, _Secound,_More...>;
	public:
		_Tuple(const _First &first, const _Secound &secound,const _More &...args)
			:m_first(first)
			,m_secound(secound)
			,m_more(args...)
		{}

		~_Tuple() = default;

		template<int index>
		stdx::type_at<index, tl_t> &get()
		{
			return m_more.get<index - 2>();
		}

		template<>
		stdx::type_at<0, tl_t> &get<0>()
		{
			return m_first;
		}

		template<>
		stdx::type_at<1, tl_t> &get<1>()
		{
			return m_secound;
		}

		template<int index>
		void set(const stdx::type_at<index, tl_t> &other)
		{
			m_more.set<index - 2>(other);
		}

		template<>
		void set<0>(const stdx::type_at<0, tl_t> &other)
		{
			m_first = other;
		}
		template<>
		void set<1>(const stdx::type_at<1, tl_t> &other)
		{
			m_secound = other;
		}
	private:
		_First m_first;
		_Secound m_secound;
		stdx::_Tuple<_More...> m_more;
	};

	//Tuple模板
	template<typename ..._T>
	class tuple
	{
		using impl_t = std::shared_ptr<stdx::_Tuple<_T...>>;
	public:
		using type_list = stdx::type_list<_T...>;
		tuple() = default;
		explicit tuple(const _T &...args)
			:m_imp(std::make_shared<stdx::_Tuple<_T...>>(args...))
		{}
		tuple(const tuple<_T...> &other)
			:m_imp(other.m_imp)
		{}
		enum
		{
			size = type_list::size
		};
		tuple(tuple<_T...> &&other)
			:m_imp(std::move(other.m_imp))
		{}
		tuple<_T...> operator=(const tuple<_T...> &other)
		{
			m_imp = other.m_imp;
			return *this;
		}
		template<int index>
		stdx::type_at<index, type_list> &get()
		{
			return m_imp->get<index>();
		}

		template<int index>
		void set(const stdx::type_at<index, type_list> &other)
		{
			return m_imp->set<index>(other);
		}

		~tuple() = default;
	private:
		impl_t m_imp;
	};
}