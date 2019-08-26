#include <stdx/net/server.h>

stdx::_ClientManager::_ClientManager()
	:m_clients()
{}

stdx::_ClientManager::~_ClientManager() noexcept
{}



void stdx::_ClientManager::add_client(const stdx::socket & client)
{
	m_clients.push_back(client);
}

void stdx::_ClientManager::del_client(const stdx::socket & client)
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

stdx::_ClientManager::client_collection_t::iterator stdx::_ClientManager::begin()
{
	return m_clients.begin();
}

stdx::_ClientManager::client_collection_t::iterator stdx::_ClientManager::end()
{
	return m_clients.end();
}

stdx::_ClientManager::client_collection_t::const_iterator stdx::_ClientManager::cbegin() const
{
	return m_clients.cbegin();
}

stdx::_ClientManager::client_collection_t::const_iterator stdx::_ClientManager::cend() const
{
	return m_clients.cend();
}

stdx::_ClientManager::client_collection_t::reverse_iterator stdx::_ClientManager::rbegin()
{
	return m_clients.rbegin();
}

stdx::_ClientManager::client_collection_t::reverse_iterator stdx::_ClientManager::rend()
{
	return m_clients.rend();
}

stdx::_ClientManager::client_collection_t::const_reverse_iterator stdx::_ClientManager::crbegin() const
{
	return m_clients.crbegin();
}

stdx::_ClientManager::client_collection_t::const_reverse_iterator stdx::_ClientManager::crend() const
{
	return m_clients.crend();
}

stdx::_ClientManager::client_collection_t::iterator stdx::_ClientManager::find(const stdx::socket & client)
{
	return std::find(std::begin(m_clients),std::end(m_clients),client);
}
