#include <iostream>
#include <list>
#include <utility>
#include <tuple>

#include <boost/asio.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/bind/bind.hpp>

#include "ConvertPacket.hpp"
#include "P2P-packet.hpp"

class FssServer;

namespace P2P
{
    class P2PMediator
    {
    public:
        using callback_t = boost::_bi::bind_t<boost::_bi::unspecified, boost::_mfi::mf2<void, FssServer, int, P2P::EndpointInfo>, boost::_bi::list3<boost::_bi::value<FssServer *>, boost::arg<1>, boost::arg<2>>>;

    private:
        callback_t m_callback;
        boost::asio::io_service m_ioservice;
        std::list<std::tuple<EndpointInfo, int, boost::chrono::system_clock::time_point>> m_connections;
        socket m_socket;
        void AddConnection(EndpointInfo endpointInfo, int otherId);
        void CleanUpConnections();
        bool SearchConnection(int peerId, int clientId, EndpointInfo &result);
        void DeleteConnection(int peerId, int clientId);

    public:
        P2PMediator(unsigned short port, callback_t callback);
        void Run();
    };
}