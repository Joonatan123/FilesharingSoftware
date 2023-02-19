#include "P2P-mediator.hpp"
void P2P::P2PMediator::AddConnection(EndpointInfo endpointInfo, int otherId)
{
    boost::chrono::system_clock::duration timeout_duration(boost::chrono::seconds(30));
    m_connections.push_back(std::tuple<EndpointInfo, int, boost::chrono::system_clock::time_point>(endpointInfo, otherId, boost::chrono::system_clock::now() + timeout_duration));
}
void P2P::P2PMediator::CleanUpConnections()
{
    for (auto connection = m_connections.begin(); connection != m_connections.end(); connection++)
    {

        if (std::get<2>(*connection) < boost::chrono::system_clock::now())
        {
            m_connections.erase(connection);
            connection = m_connections.begin(); // quick hack to avoid segfault
            // std::cout << "erased outdated udp connection\n";
        }
    }
}
bool P2P::P2PMediator::SearchConnection(int peerId, int clientId, EndpointInfo &result)
{
    for (auto connection = m_connections.begin(); connection != m_connections.end(); connection++)
    {
        if (std::get<2>(*connection) > boost::chrono::system_clock::now() && std::get<0>(*connection).endpointOwnerId == peerId && std::get<1>(*connection) == clientId)
        {
            result = std::get<0>(*connection);
            return true;
        }
    }
    return false;
}
void P2P::P2PMediator::DeleteConnection(int peerId, int clientId)
{

    for (auto connection = m_connections.begin(); connection != m_connections.end(); connection++)
    {

        if (std::get<0>(*connection).endpointOwnerId == peerId && std::get<1>(*connection) == clientId)
        {
            m_connections.erase(connection);
            connection = m_connections.begin(); // quick hack to avoid segfault
            LOG_TRACE(p2p) << "erased cause already connected";
        }
    }
    for (auto connection = m_connections.begin(); connection != m_connections.end(); connection++)
    {

        if (std::get<0>(*connection).endpointOwnerId == clientId && std::get<1>(*connection) == peerId)
        {
            m_connections.erase(connection);
            connection = m_connections.begin(); // quick hack to avoid segfault
            LOG_TRACE(p2p) << "erased cause already connected\n";
        }
    }
}

P2P::P2PMediator::P2PMediator(unsigned short port, callback_t callback) : m_socket(m_ioservice, endpoint(udp::v4(), port)), m_callback(callback)
{
}
void P2P::P2PMediator::Run()
{
    for (;;)
    {
        char read_buffer[1000] = {0};
        endpoint clientEndpoint;
        m_socket.receive_from(boost::asio::buffer(read_buffer), clientEndpoint);

        OpenConnectionPacket openConnectionPacket;
        ConvertBufferToMessage(read_buffer, openConnectionPacket);

        EndpointInfo clientEndpointInfo(openConnectionPacket.senderId, clientEndpoint);

        // clientEndpointInfo.Print();

        AddConnection(clientEndpointInfo, openConnectionPacket.peerId);
        EndpointInfo peerEndpointInfo;

        CleanUpConnections();
        if (SearchConnection(openConnectionPacket.peerId, openConnectionPacket.senderId, peerEndpointInfo))
        {
            std::string message = ConvertMessageToString(clientEndpointInfo);
            // endpoint peerEndpoint = peerEndpointInfo.ToEndpoint();
            // m_socket.send_to(boost::asio::buffer(message), peerEndpoint);
            m_callback(openConnectionPacket.senderId, peerEndpointInfo);
            m_callback(openConnectionPacket.peerId, clientEndpointInfo);

            DeleteConnection(openConnectionPacket.peerId, openConnectionPacket.senderId);

            // message = ConvertMessageToString(peerEndpointInfo);
            // m_socket.send_to(boost::asio::buffer(message), clientEndpoint);
        }
    }
}