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

void stdx::basic_event_reactor::register_client(stdx::socket && client)
{
	auto iterator = m_parser_table.find(client);
	auto end = m_parser_table.end();
	if (iterator != end)
	{
		return;
	}
	if (!on_connect(client))
	{
		return;
	}
	auto parser = this->make_parser();
	m_parser_table.emplace(client, parser);
	auto f= [client, this, parser](stdx::network_recv_event &&ev) mutable
	{
		//handle data
		stdx::parse_process process = parser.parse(ev.buffer, ev.size);
		if (process == stdx::parse_process::complete)
		{
			on_recv(client, parser.get_package());
			return;
		}
		if (process == stdx::parse_process::over)
		{
			for (size_t i = 0, count = parser.get_packages_count(); i < count; i++)
			{
				on_recv(client, parser.get_package());
			}
			return;
		}
		if (process == stdx::parse_process::error)
		{
			parser.complete();
			if (parser.get_packages_count())
			{
				on_recv(client,parser.get_package());
			}
			return;
		}

	};
	auto er = [client, this](std::exception_ptr err) mutable
	{
		//handle error
		if (on_error(client, err))
		{
			unregister_client(client);
			on_disconnect(client);
		}
	};;
	client.recv_utill_error(4096,std::move(f),std::move(er));
}

void stdx::basic_event_reactor::unregister_client(const stdx::socket & client)
{
	auto iterator = m_parser_table.find(client);
	auto end = m_parser_table.end();
	if (iterator != end)
	{
		m_parser_table.erase(client);
	}
}

void stdx::_TcpServer::run(cstring ip, const uint_16 & port)
{
	if (m_running)
	{
		m_server_socket.close();
	}
	else
	{
		m_running = true;
	}
	m_server_socket.init(stdx::addr_family::ip, stdx::socket_type::stream, stdx::protocol::tcp);
	stdx::network_addr addr(ip, port);
	m_server_socket.bind(addr);
	m_server_socket.listen(5);
	while (m_running)
	{
		auto client = m_server_socket.accept();
		m_reactor.register_client(std::move(client));
	}
}

void stdx::_TcpServer::close() noexcept
{
	if (m_running)
	{
		m_running = false;
		m_reactor.clear();
		m_server_socket.close();
	}
}
