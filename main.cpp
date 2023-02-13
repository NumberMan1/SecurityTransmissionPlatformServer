#include <iostream>
#include <fstream>
#include <json/json.h>
#include "Server.h"
#include <openssl/mine_hash.hpp>
#include <spdlog/spdlog.h>
#include <chrono>

int main(int argc, char *argv[]) {
    using namespace boost::asio;    spdlog::info("Welcome to spdlog!");
    std::ifstream in {"for_server/server.json"};
    Json::Reader reader {};
    Json::Value root {};
    reader.parse(in, root);
    std::string server_id = root["server_id"].asString();
    std::uint16_t port = root["port"].asUInt();
    try {
        std::cout << "服务器:" << server_id << "启动\n"
                  << "端口为:" << port << "\n";
        // boost::asio::io_service io_service;
        // platform::ServerOP server(
        //     io_service,
        //     ip::tcp::endpoint(ip::tcp::v4(), port),
        //     server_id
        // );
        // io_service.run();
        
        platform::Server server(server_id, port);
        server.Start();
        while (true) {
            server.Update(1);
        }
    } catch (std::exception &err) {
        std::cerr << "错误为:" << err.what() << std::endl;
    }
    return 0;
}