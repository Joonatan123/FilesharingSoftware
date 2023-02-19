#include "network-server.hpp"

NetworkServer::NetworkServer(callback_t callback) : m_ioservice(), m_acceptor(m_ioservice), m_connections(), m_callback(callback) {}

void NetworkServer::handle_read(con_handle_t con_handle, boost::system::error_code const &err, size_t bytes_transfered)
{
	if (err)
	{
		if (err == boost::asio::error::eof)
			LOG_INFO(network) << "connection closed";
		else
			LOG_ERROR(network) << "erron on read handle: " << err.message();
		m_callback(Header(), nullptr, con_handle, err);
		return;
	}
	if (bytes_transfered > 0)
	{
		// std::string archive_header_data(std::string(std::istreambuf_iterator<char>(&con_handle->read_buffer), std::istreambuf_iterator<char>()));
		std::string archive_header_data;
		std::copy_n(boost::asio::buffers_begin(con_handle->read_buffer.data()), bytes_transfered, back_inserter(archive_header_data));
		con_handle->read_buffer.consume(bytes_transfered);
		// std::cout << archive_header_data << std::endl;
		std::istringstream archive_header_stream(archive_header_data);
		Header header;
		std::string data_string;
		{
			try
			{
				boost::archive::text_iarchive archive_header(archive_header_stream, boost::archive::archive_flags::no_header);

				archive_header >> header;
				LOG_PACKET << Header::TypeToString(header.message_type);
				m_callback(header, &archive_header, con_handle, err);
			}
			catch (std::exception &e)
			{
				LOG_ERROR(network) << "And error occurred in packet handling\n";
			}
		}
		// std::cout << data_string << std::endl;
	}

	if (!err)
	{
		do_async_read(con_handle);
	}
	else
	{
		m_callback(Header(), nullptr, con_handle, err);
		LOG_ERROR(network) << "error in handle read: " << err.message() << std::endl;
	}
}

void NetworkServer::do_async_read(con_handle_t con_handle)
{
	auto handler = boost::bind(&NetworkServer::handle_read, this, con_handle, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);
	// boost::asio::async_read(con_handle->socket, con_handle->read_buffer, handler); // sizeof(Header)
	boost::asio::async_read_until(con_handle->socket, con_handle->read_buffer, "\n", handler);
}

void NetworkServer::handle_write(con_handle_t con_handle, boost::system::error_code const &err)
{
	if (!err)
	{
		if (con_handle->socket.is_open())
		{
			// Write completed successfully and connection is open
		}
	}
	else
	{
		std::cerr << "write error: " << err.message() << std::endl;
		// m_connections.erase(con_handle);
	}
}
/*template <typename T>
void NetworkServer::send_packet(con_handle_t con_handle, Header header, T message)
{
	auto handler = boost::bind(&NetworkServer::handle_write, this, con_handle, boost::asio::placeholders::error);
	Network::send_packet(*con_handle, header, message, handler);
}*/
// used for redirect
void NetworkServer::send_string(con_handle_t con_handle, std::string message)
{
	auto handler = boost::bind(&NetworkServer::handle_write, this, con_handle, boost::asio::placeholders::error);
	Network::send_string(*con_handle, message, handler);
}

void NetworkServer::handle_accept(con_handle_t con_handle, boost::system::error_code const &err)
{
	if (!err)
	{
		con_handle->socket.remote_endpoint().address();
		// send_some(con_handle, err);
		do_async_read(con_handle);
	}
	else
	{
		LOG_ERROR(network) << "error in handle accept: " << err.message() << std::endl;
		m_connections.erase(con_handle);
	}
	start_accept();
}

void NetworkServer::start_accept()
{
	auto con_handle = m_connections.emplace(m_connections.begin(), m_ioservice);
	auto handler = boost::bind(&NetworkServer::handle_accept, this, con_handle, boost::asio::placeholders::error);
	m_acceptor.async_accept(con_handle->socket, handler);
}

void NetworkServer::listen(uint16_t port)
{
	auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port);
	m_acceptor.open(endpoint.protocol());
	m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	m_acceptor.bind(endpoint);
	m_acceptor.listen();
	start_accept();
}

void NetworkServer::run()
{
	m_ioservice.run();
}
