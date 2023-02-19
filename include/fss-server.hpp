#include <iostream>
#include <boost/assign/std/map.hpp>

#include "network-server.hpp"
#include "fss-class.hpp"
#include "P2P-mediator.hpp"
class FssServer
{
    P2P::P2PMediator m_mediator;
    NetworkServer m_network;
    std::vector<std::string> m_cmdbuffer;

    using sys_handle_t = std::list<ClientSystem>::iterator;

    std::list<ClientSystem> m_clients;
    std::map<unsigned long, sys_handle_t> m_id_clientmap;
    std::map<unsigned long, NetworkServer::con_handle_t> m_id_conmap;

    struct ConHandleComparator
    {
        bool operator()(const NetworkServer::con_handle_t &a, const NetworkServer::con_handle_t &b) const
        {
            return &(*a) > &(*b);
        }
    };

    std::map<NetworkServer::con_handle_t, sys_handle_t, ConHandleComparator> m_con_clientmap;

    unsigned long add_client(NetworkServer::con_handle_t con_handle, unsigned long p_id = 0);
    unsigned long FindId(NetworkServer::con_handle_t con_handle);
    // when a entry is updated send the update to all other clients
    void SendSingleUpdateToAll(unsigned long id_excluded);
    void send_full_peer_update(NetworkServer::con_handle_t con_handle, unsigned long id);

    void SaveState();
    void LoadState();

public:
    FssServer();
    NetworkServer::callback_t make_callback();
    P2P::P2PMediator::callback_t make_mediator_callback();
    void handle_packet(Header header, PacketBinary *packet, NetworkServer::con_handle_t con_handle, boost::system::error_code const &err);
    void handle_mediator_write(int receiverId, P2P::EndpointInfo endPointInfo);
    void start();
};
int main();