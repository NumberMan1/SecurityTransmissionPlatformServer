#ifndef PLATFORM_SERVER_H
#define PLATFORM_SERVER_H

#include <string>
#include <boost/asio/thread_pool.hpp>

#include "net/net_client_server.hpp"
#include "factory.h"

namespace platform {

class Server : public mine_net::ServerInterface<transmission_msg::NetMsgType> {
public:
    using TMsgType = transmission_msg::NetMsgType;
    Server(std::uint16_t port) 
        : ServerInterface(port) {
        
    }
    // 在验证客户端时调用
    virtual void OnClientValidated
        (std::shared_ptr<mine_net::Connection<TMsgType>> client) override  {
    }
protected:

    // 当客户端连接时调用，可以通过返回false否决连接
    virtual bool OnClientConnect
        (std::shared_ptr<mine_net::Connection<TMsgType>> client) override  {
        return true;
    }

    // 当客户端似乎已断开连接时调用
    virtual void OnClientDisconnect
        (std::shared_ptr<mine_net::Connection<TMsgType>> client) override  {
        client->Disconnect();
    }

    // 收到消息时调用
    virtual void OnMessage
        (std::shared_ptr<mine_net::Connection<TMsgType>> client,
         mine_net::Message<TMsgType>& msg) override  {
        // 利用线程池进行回应
        boost::asio::post(
            thread_pool_,
            std::bind(
                &Server::Work, this,
                client, msg
            )
        );
    }
private:
    bool CheckSign(const std::string_view &pubkey_file_name);
    bool SeckeyAgree(std::shared_ptr<mine_net::Connection<TMsgType>> client,
                     mine_net::Message<TMsgType> &msg);
    bool SeckeyVerify(std::shared_ptr<mine_net::Connection<TMsgType>> client,
                      mine_net::Message<TMsgType> &msg);
    bool SeckeyLogout(std::shared_ptr<mine_net::Connection<TMsgType>> client,
                      mine_net::Message<TMsgType> &msg);
    void Work(std::shared_ptr<mine_net::Connection<TMsgType>> client,
              mine_net::Message<TMsgType> &msg);
    boost::asio::thread_pool thread_pool_ {4};
};
// class Session {
// public:
//     constexpr static std::uint32_t kMaxNetMsgLen = 1024;
//     inline boost::asio::ip::tcp::socket& socket() {
//         return socket_;
//     }
//     explicit Session(boost::asio::io_service &io_service);
//     void Start();
//     // async_read用于处理消息头部
//     // void HandleReadHeader(const boost::system::error_code &error);
//     // async_read用于处理消息主体
//     void HandleReadBody(const boost::system::error_code &error);
//     void HandleSend(const boost::system::error_code &error);
//     bool SeckeyAgree();
//     bool SeckeyVerify();
//     bool SeckeyLogout();
// private:
//     boost::asio::ip::tcp::socket socket_;
//     // transmission_msg::net::Message<NetMsgType> msg_;
//     std::vector<char> buf_;
// };
// class ServerOP {
// public:
//     explicit ServerOP(boost::asio::io_service &io_service,
//                       const boost::asio::ip::tcp::endpoint &endpoint,
//                       const std::string &server_id);
//     // 用于处理async_accept
//     void HandleAccept(std::shared_ptr<Session> session_ptr,
//                       const boost::system::error_code &error);

// private:
//     boost::asio::io_service &io_service_;
//     boost::asio::ip::tcp::acceptor acceptor_;
//     std::uint16_t port_;
//     std::string server_id_;
//     boost::asio::thread_pool thread_pool_;
// };


} // namespace platform

#endif // SERVEROP_H