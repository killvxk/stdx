#include <stdx/net/server.h>

stdx::basic_client_manager::basic_client_manager()
	:m_clients()
{}

stdx::basic_client_manager::~basic_client_manager() noexcept
{}



void stdx::basic_client_manager::add_client(const stdx::socket & client)
{
	m_clients.push_back(client);
}

void stdx::basic_client_manager::del_client(const stdx::socket & client)
{
	auto end = std::end(m_clients);
	auto iterator = std::find(std::begin(m_clients), end, client);
	if (iterator == end)
	{
		return;
	}
	stdx::socket sock = *iterator;
	m_clients.erase(iterator);
	sock.close();
}

stdx::basic_client_manager::client_collection_t::iterator stdx::basic_client_manager::begin()
{
	return m_clients.begin();
}

stdx::basic_client_manager::client_collection_t::iterator stdx::basic_client_manager::end()
{
	return m_clients.end();
}

stdx::basic_client_manager::client_collection_t::const_iterator stdx::basic_client_manager::cbegin() const
{
	return m_clients.cbegin();
}

stdx::basic_client_manager::client_collection_t::const_iterator stdx::basic_client_manager::cend() const
{
	return m_clients.cend();
}

stdx::basic_client_manager::client_collection_t::reverse_iterator stdx::basic_client_manager::rbegin()
{
	return m_clients.rbegin();
}

stdx::basic_client_manager::client_collection_t::reverse_iterator stdx::basic_client_manager::rend()
{
	return m_clients.rend();
}

stdx::basic_client_manager::client_collection_t::const_reverse_iterator stdx::basic_client_manager::crbegin() const
{
	return m_clients.crbegin();
}

stdx::basic_client_manager::client_collection_t::const_reverse_iterator stdx::basic_client_manager::crend() const
{
	return m_clients.crend();
}

stdx::basic_client_manager::client_collection_t::iterator stdx::basic_client_manager::find(const stdx::socket & client)
{
	return std::find(std::begin(m_clients),std::end(m_clients),client);
}

void stdx::basic_client_manager::clear()
{
	m_clients.clear();
}
