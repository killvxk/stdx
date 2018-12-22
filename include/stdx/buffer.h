#pragma once
#include <memory>
namespace stdx
{
	using byte = unsigned char;
	template<int size>
	class buffer
	{
	public:
		buffer()
			:m_allocator(std::make_shared<std::allocator<byte>>())
			,m_ptr(m_allocator->allocate(size))
		{
		}

		~buffer()
		{
			m_allocator->unallocate(m_ptr,size);
		}

		int size() const
		{
			return size;
		}

		const byte *c_str() const
		{
			return m_ptr;
		}

		byte operator[](const int& i)
		{
			return m_ptr[i];
		}

	private:
		std::shared_ptr<std::allocator<_T>> m_allocator;
		byte *m_ptr;
	};
}