#ifndef PLATFORM_SERVER_H
#define PLATFORM_SERVER_H

#include <string>
#include <boost/asio.hpp>
#include <boost/thread/thread_pool.hpp>

namespace platform {
    
class Session {
public:
    inline boost::asio::ip::tcp::socket& socket() {
        return socket_;
    }
    explicit Session(boost::asio::io_service &io_service);
    void Start();
private:
    boost::asio::ip::tcp::socket socket_;
};
class ServerOP {
public:
    explicit ServerOP(boost::asio::io_service &io_service,
                      const boost::asio::ip::tcp::endpoint &endpoint,
                      const std::string &server_id);
    void HandleAccept(std::shared_ptr<Session> session_ptr,
                      const boost::system::error_code &error);
    bool SeckeyAgree();
    bool SeckeyVerify();
    bool SeckeyLogout();
private:
    boost::asio::io_service &io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::uint16_t port_;
    std::string server_id_;
    boost::asio::thread_pool thread_pool_;
};


} // namespace platform

#endif // SERVEROP_H