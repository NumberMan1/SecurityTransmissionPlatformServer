#include <iostream>
#include <fstream>
#include <json/json.h>
#include "Server.h"
#include "msg.h"
#include "net/net_connection.hpp"
#include "net/net_client_server.hpp"
using namespace mine_net;
class S : public ServerInterface<int> {
protected:
    // 这个server类应该覆盖这些函数来实现
    // 自定义功能

    // 当客户端连接时调用，可以通过返回false否决连接
    virtual bool OnClientConnect(std::shared_ptr<Connection<int>> client)  {
        return true;
    }

    // 当客户端似乎已断开连接时调用
    virtual void OnClientDisconnect(std::shared_ptr<Connection<int>> client)  {
		
    }

    // 收到消息时调用
    virtual void OnMessage(std::shared_ptr<Connection<int>> client, Message<int>& msg)  {
        std::cout << msg << '\n';
        client->Send(msg);
    }

public:
    S(std::uint16_t port) 
        : ServerInterface(port) {
        
    }
    // 在验证客户端时调用
    virtual void OnClientValidated(std::shared_ptr<Connection<int>> client)  {
        Message<int> msg;
        msg.header.id = 1;
        client->Send(msg);
    }
};

int main(int argc, char *argv[]) {
    using namespace boost::asio;
    std::ifstream in {"server.json"};
    Json::Reader reader {};
    Json::Value root {};
    reader.parse(in, root);
    std::string server_id = root["server_id"].asString();
    std::uint16_t port = root["port"].asUInt();
    try {
        // boost::asio::io_service io_service;
        // platform::ServerOP server(
        //     io_service,
        //     ip::tcp::endpoint(ip::tcp::v4(), port),
        //     server_id
        // );
        // std::cout << "服务器:" << server_id << "启动\n"
        //           << "端口为:" << port << "\n";
        // io_service.run();
        thread_pool pool(4);
        post([port](){
            S se(port);
            se.Start();
            while (true) {
                se.Update(-1);
            }
        });
    } catch (std::exception &err) {
        std::cerr << "错误为:" << err.what() << std::endl;
    }
    return 0;
}