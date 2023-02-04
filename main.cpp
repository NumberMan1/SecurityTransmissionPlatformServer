#include <iostream>
#include <fstream>
#include <json/json.h>
#include "Server.h"
#include <openssl/mine_hash.hpp>
#include <spdlog/spdlog.h>

int main(int argc, char *argv[]) {
    using namespace boost::asio;    spdlog::info("Welcome to spdlog!");
    std::ifstream in {"for_server/server.json"};
    Json::Reader reader {};
    Json::Value root {};
    reader.parse(in, root);
    std::string server_id = root["server_id"].asString();
    std::uint16_t port = root["port"].asUInt();
    using namespace mine_openssl;
    std::cout.setf(std::ios_base::boolalpha);
    // Hash h1(HashType::kSHA256Type), h2(HashType::kSHA384Type), h3(HashType::kSHA512Type);
    // std::cout << h1.Final() << "\n" << h2.Final() << "\n" << h3.Final() << "\n";
    // h1.Update(server_id), h2.Update(server_id), h3.Update(server_id);
    // std::cout << h1.Final() << "\n" << h2.Final() << "\n" << h3.Final() << "\n";
    // h1.Update(server_id), h2.Update(server_id), h3.Update(server_id);
    // std::cout << h1.Final() << "\n" << h2.Final() << "\n" << h3.Final() << "\n";
    
    // h1.Update(server_id);
    // std::string s = h1.Final(), b64, re;
    // b64 = ToBase64(s);
    // re = FromBase64(b64);
    // std::cout << b64 << '\n';
    // std::cout << s << '\n';
    // std::cout << re << '\n';
    MyRSA rsa("for_server/server_pubkey.pem", "for_server/server_prikey.pem");
    auto enc = rsa.EncryptPubKey(server_id);
    auto dec = rsa.DecryptPriKey(enc);
    std::cout << server_id << "\n\n" << enc << "\n\n" << dec << "\n\n" << "\n";
    auto sig = rsa.Sign(server_id, MyRSA::SignType::kSHA256Type);
    auto flag = rsa.Verify(server_id, sig, MyRSA::SignType::kSHA256Type);
    std::cout << sig << "\n\n" << flag << std::endl;
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
        
        // platform::Server server(server_id, port);
        // server.Start();
        // while (true) {
        //     server.Update();
        // }
    } catch (std::exception &err) {
        std::cerr << "错误为:" << err.what() << std::endl;
    }
    return 0;
}