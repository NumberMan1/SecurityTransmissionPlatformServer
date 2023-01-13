#ifndef PLATFORM_SERVER_H
#define PLATFORM_SERVER_H

#include <string>
#include <boost/asio.hpp>
#include <boost/thread/thread_pool.hpp>

#include "msg.h"

namespace platform {

enum class NetMsgType {
    kRequestMsg,
    kRespondMsg
};

class Session {
public:
    constexpr static std::uint32_t kMaxNetMsgLen = 1024;
    inline boost::asio::ip::tcp::socket& socket() {
        return socket_;
    }
    explicit Session(boost::asio::io_service &io_service);
    void Start();
    // async_read用于处理消息头部
    // void HandleReadHeader(const boost::system::error_code &error);
    // async_read用于处理消息主体
    void HandleReadBody(const boost::system::error_code &error);
    void HandleSend(const boost::system::error_code &error);
    bool SeckeyAgree();
    bool SeckeyVerify();
    bool SeckeyLogout();
private:
    boost::asio::ip::tcp::socket socket_;
    // transmission_msg::net::Message<NetMsgType> msg_;
    std::vector<char> buf_;
};
class ServerOP {
public:
    explicit ServerOP(boost::asio::io_service &io_service,
                      const boost::asio::ip::tcp::endpoint &endpoint,
                      const std::string &server_id);
    // 用于处理async_accept
    void HandleAccept(std::shared_ptr<Session> session_ptr,
                      const boost::system::error_code &error);

private:
    boost::asio::io_service &io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::uint16_t port_;
    std::string server_id_;
    boost::asio::thread_pool thread_pool_;
};


} // namespace platform

#endif // SERVEROP_H