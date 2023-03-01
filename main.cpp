#include <iostream>
#include <fstream>
#include <json/json.h>
#include "Server.h"
#include <openssl/mine_hash.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <chrono>
#include "mysql_seckey.hpp"

#include "seckey_shm.hpp"

int main(int argc, char *argv[]) {
    using namespace boost::asio;
    std::ifstream is{"for_server/server.json"};
    Json::Value root{};
    is >> root;
    std::string server_id = root["server_id"].asString();
    std::uint16_t port = root["port"].asUInt();
    is.close();
    // using namespace boost::interprocess;
    // int fd = fork();
    // if (fd == 0) {
    //     using namespace std::chrono_literals;
    //     std::this_thread::sleep_for(2s);
    //     shared_memory_object sm{open_only, "text_shared", read_only};
    //     mapped_region mapped_r(sm, read_only);
    //     std::cout << "address : " << mapped_r.get_address() << "\n";
    //     std::cout << "size : " << mapped_r.get_size() << "\n";
    //     std::string *result = static_cast<std::string*>(mapped_r.get_address());
    //     std::cout << "sm has " << *result << std::endl;
    //     std::cout << shared_memory_object::remove("text_shared");
    //     return 0;
    // } else {
    //     shared_memory_object::remove("text");
    //     shared_memory_object::remove("text_shared");
    //     shared_memory_object sm{create_only, "text_shared", read_write};
    //     sm.truncate(1024);
    //     mapped_region mapped_r(sm, read_write);
    //     std::cout << "address : " << mapped_r.get_address() << "\n";
    //     std::cout << "size : " << mapped_r.get_size() << "\n";
    //     std::string *result = static_cast<std::string*>(mapped_r.get_address());
    //     *result = "hello";
    //     std::cout << "sm has " << *result << std::endl;
    //     return 0;
    // }
    try {
        std::cout << "服务器:" << server_id << "启动\n"
                  << "端口为:" << port << "\n";

        // 设置工作路径，初始化环境
        if (!std::filesystem::exists("for_server")) {
            std::filesystem::create_directory("for_server");
        }
        std::filesystem::current_path("for_server");
        if (!std::filesystem::exists("seckey")) {
            std::filesystem::create_directory("seckey");
        }
        // 只要一个密钥不存在就重新生成
        if (!std::filesystem::exists("server_pubkey.pem") 
            || !std::filesystem::exists("server_prikey.pem")) {
            mine_openssl::MyRSA rsa(1);
            rsa.SaveRSAKey("server_pubkey.pem", "server_prikey.pem");
        }

        platform::Server server(server_id, port);
        server.Start();
        while (true) {
            server.Update(1);
        }
    } catch (std::exception &err) {
        spdlog::error(err.what());
        auto logger = spdlog::daily_logger_mt("int main logger", "log/int_main");
        logger->error(err.what());
        // std::cerr << "错误为:" << err.what() << std::endl;
    } catch (...) {
        spdlog::error("未知的错误");
        auto logger = spdlog::daily_logger_mt("int main logger", "log/int_main");
        logger->error("未知的错误");
    }
    return 0;
}