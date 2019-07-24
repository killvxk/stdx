#include <stdx/io.h>

stdx::_Buffer::_Buffer(size_t size)
	:m_size(size)
	, m_data((char*)calloc(sizeof(char), m_size))
{}

stdx::_Buffer::_Buffer(size_t size, char* data)
	:m_size(size)
	,m_data(data)
{}

stdx::_Buffer::~_Buffer()
{
	if (m_data)
	{
		free(m_data);
	}
}

char &stdx::_Buffer::operator[](const size_t &i) const
{
	if (i >= m_size)
	{
		throw std::out_of_range("out of range");
	}
	return *(m_data + i);
}
void stdx::_Buffer::realloc(size_t size)
{
	if (size == 0)
	{
		throw std::invalid_argument("invalid argument: 0");
	}
	if (size > m_size)
	{
		if (::realloc(m_data, m_size) == nullptr)
		{
			throw std::bad_alloc();
		}
		m_size = size;
	}
}

void stdx::_Buffer::copy_from(const stdx::_Buffer &other)
{
	auto new_size = other.size();
	if (new_size > m_size)
	{
		realloc(new_size);
	}
	memcpy(m_data, other, new_size);
}