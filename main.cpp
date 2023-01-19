#include <iostream>
#include <fstream>
#include <json/json.h>
#include "Server.h"
#include <openssl/mine_hash.h>

int main(int argc, char *argv[]) {
    using namespace boost::asio;
    std::ifstream in {"for_server/server.json"};
    Json::Reader reader {};
    Json::Value root {};
    reader.parse(in, root);
    std::string server_id = root["server_id"].asString();
    std::uint16_t port = root["port"].asUInt();
    using namespace mine_openssl;
    Hash h1(HashType::kSHA256Type), h2(HashType::kSHA384Type), h3(HashType::kSHA512Type);
    std::cout << h1.Final() << "\n" << h2.Final() << "\n" << h3.Final() << "\n";
    h1.Update(server_id), h2.Update(server_id), h3.Update(server_id);
    std::cout << h1.Final() << "\n" << h2.Final() << "\n" << h3.Final() << "\n";
    h1.Update(server_id), h2.Update(server_id), h3.Update(server_id);
    std::cout << h1.Final() << "\n" << h2.Final() << "\n" << h3.Final() << "\n";
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
        platform::Server server(server_id, port);
        server.Start();
        while (true) {
            server.Update();
        }
    } catch (std::exception &err) {
        std::cerr << "错误为:" << err.what() << std::endl;
    }
    return 0;
}