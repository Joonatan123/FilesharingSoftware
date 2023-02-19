#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/thread/thread.hpp>

#include <cstdint>
#include <iostream>
#include <list>
#include <memory>

#include "network.hpp"
#include "file-list.hpp"
#include "log.hpp"
struct FssServer;

class NetworkServer
{
public:
	using con_handle_t = std::list<Connection>::iterator;
	using callback_t = boost::_bi::bind_t<boost::_bi::unspecified, boost::_mfi::mf4<void, FssServer, Header, PacketBinary *, NetworkServer::con_handle_t, const boost::system::error_code &>, boost::_bi::list5<boost::_bi::value<FssServer *>, boost::arg<1>, boost::arg<2>, boost::arg<3>, boost::arg<4>>>;

private:
	boost::asio::io_service m_ioservice;
	boost::asio::ip::tcp::acceptor m_acceptor;
	std::list<Connection> m_connections;
	callback_t m_callback;

public:
	NetworkServer(callback_t callback);

	void handle_read(con_handle_t con_handle, boost::system::error_code const &err, size_t bytes_transfered);

	void do_async_read(con_handle_t con_handle);

	void handle_write(con_handle_t con_handle, boost::system::error_code const &err);

	template <typename T>
	void send_packet(con_handle_t con_handle, Header header, T message)
	{
		auto handler = boost::bind(&NetworkServer::handle_write, this, con_handle, boost::asio::placeholders::error);
		Network::send_packet(*con_handle, header, message, handler);
	}
	// used for redirect
	void send_string(con_handle_t con_handle, std::string message);

	void handle_accept(con_handle_t con_handle, boost::system::error_code const &err);

	void start_accept();

	void listen(uint16_t port);

	void run();
};
