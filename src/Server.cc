#include "Server.h"
#include <iostream>
#include <fstream>

#include "openssl/mine_hash.hpp"

platform::Server::Server(std::string server_id, std::uint16_t port)
    : ServerInterface(port),
      server_id_(server_id) {
    // 设置工作路径，初始化环境
    std::error_code err;
    if (!std::filesystem::exists("for_server", err)) {
        std::filesystem::create_directory("for_server", err);
    }
    std::filesystem::current_path("for_server", err);
    if (!std::filesystem::exists("seckey", err)) {
        std::filesystem::create_directory("seckey", err);
    }
    // 只要一个密钥不存在就重新生成
    if (!std::filesystem::exists("server_pubkey.pem")) {
        mine_openssl::MyRSA rsa(1);
        rsa.SaveRSAKey("server_pubkey.pem", "server_prikey.pem");
    }
    if (err) {
        std::cout << err.message() << "\n";
    }
}

// bool platform::Server::CheckSign(const std::string_view &pubkey_file_name,
//                                  const std::string_view &data,
//                                  const std::string_view &sign_data) {
//     mine_openssl::MyRSA rsa(pubkey_file_name);
//     return rsa.Verify(data, sign_data);
// }

bool platform::Server::SeckeyAgree
    (std::shared_ptr<mine_net::Connection<TMsgType>> client,
     mine_net::Message<TMsgType> &msg) {
    // 进行提取
    std::size_t net_msg_len = msg.Size();
    std::string net_str;
    for (std::size_t i = 0; i != net_msg_len; ++i) {
        char c;
        msg >> c;
        net_str += c;
    }
    // // 由于Message是后入先出，需要修正消息
    // std::reverse(net_str.begin(), net_str.end());
    std::cout << net_str << '\n';
    using TInfo = transmission_msg::proto::Info;
    using TRTInfo = transmission_msg::proto::RequestInfo;
    using TRPInfo = transmission_msg::proto::RespondInfo;
    using TFactory = transmission_msg::proto::factory::Factory;
    using TRTFactory = transmission_msg::proto::factory::RequestFactory;
    using TRPFactory = transmission_msg::proto::factory::RespondFactory;
    TRTFactory requset_factory {net_str};
    auto request_msg = requset_factory.CreateMsg();
    auto request_info = request_msg->DecodeMsg();
    std::string client_id_pubkey_file_name = request_info->client_id;
    client_id_pubkey_file_name += "_pubkey.pem";
    client_id_pubkey_file_name.insert(0, "seckey/", 0, std::strlen("seckey/"));
    // 写入磁盘
    std::ofstream file_out(client_id_pubkey_file_name);
    file_out << request_info->data;
    // 验证签名
    mine_openssl::Hash h(mine_openssl::HashType::kSHA512Type);
    h.Update(request_info->data);
    if (!CheckSign(client_id_pubkey_file_name,
            h.Final(), request_info->sign)) {
        std::cout << "签名校验失败" << '\n';
        std::filesystem::remove(client_id_pubkey_file_name);
        return false;
    }
    // 准备传输数据
    mine_net::Message<TMsgType> msg_out;
    msg_out.header.id = TMsgType::kSeckeyAgree;
    // 生成随机字符串, 对称加密的密钥
    std::string aes_key = GetRandStr(AESKeyLen::kLen16);
    mine_openssl::MyRSA rsa(client_id_pubkey_file_name);
    // 公钥加密
    std::string aes_seckey = rsa.EncryptPubKey(aes_key);
    // 准备数据
    TRPInfo respond_info {
        TInfo {
            request_info->client_id,
            server_id_,
            aes_seckey
        },
        12 // 需要数据库操作
    };
    TRPFactory respond_factory {&respond_info};
    auto respond_msg = respond_factory.CreateMsg();
    std::string respond_str(respond_msg->EncodeMsg());
    for (auto i = std::ssize(respond_str) - 1; i != -1; --i) {
        // char c = respond_str.at(i);
        // msg_out << c;
        msg_out << respond_str.at(i);
    }
    std::cout << respond_str << "\n";
    // 发送
    client->Send(msg_out);
    // for (std::size_t i = 0; i != net_str.size(); ++i) {
    //     msg << net_str.at(i);
    // }
    // client->Send(msg);
    return true;
}

bool platform::Server::SeckeyVerify
    (std::shared_ptr<mine_net::Connection<TMsgType>> client,
     mine_net::Message<TMsgType> &msg) {
    return true;
}

bool platform::Server::SeckeyLogout
    (std::shared_ptr<mine_net::Connection<TMsgType>> client,
     mine_net::Message<TMsgType> &msg) {
    return true;
}

void platform::Server::Work(std::shared_ptr<mine_net::Connection<TMsgType>> client,
                            mine_net::Message<TMsgType> &msg) {
    switch (msg.header.id) {
    case TMsgType::kSeckeyAgree : {
        if (!SeckeyAgree(client, msg)) {
            mine_net::Message<TMsgType> failed_msg;
            failed_msg.header.id = TMsgType::kFailed;
            client->Send(failed_msg);
        }
        break;
    }
    case TMsgType::kSeckeyVerify: {        
        if (!SeckeyVerify(client, msg)) {
            mine_net::Message<TMsgType> failed_msg;
            failed_msg.header.id = TMsgType::kFailed;
            client->Send(failed_msg);
        }
        break;
    }
    case TMsgType::kSeckeyLogout: {
        if (!SeckeyLogout(client, msg)) {
            mine_net::Message<TMsgType> failed_msg;
            failed_msg.header.id = TMsgType::kFailed;
            client->Send(failed_msg);
        }
        break;   
    }
    default:
        break;
    }
}

std::string platform::Server::GetRandStr(AESKeyLen len) {
    srand(time(nullptr));	// 以当前时间为种子
    std::string retStr;
    char* buf = "~`@#$%^&*()_+=-{}[];':";
    for (std::int8_t i = 0; i < len; ++i) {
        std::int8_t flag = rand() % 4;
        switch (flag) {
        case 0:	// 0-9
            retStr.append(1, rand() % 10 + '0');
            break;
        case 1:	// a-z
            retStr.append(1, rand() % 26 + 'a');
            break;
        case 2:	// A-Z
            retStr.append(1, rand() % 26 + 'A');
            break;
        case 3:	// 特殊字符
            retStr.append(1, buf[rand() % strlen(buf)]);
            break;
        }
    }
    return retStr;
}

// platform::Session::Session(boost::asio::io_service &io_service)
//     : socket_(io_service) {
//     // msg_.body.resize(kMaxNetMsgLen);
//     // msg_.header.size = kMaxNetMsgLen;
//     buf_.resize(kMaxNetMsgLen);
// }

// void platform::Session::Start() {
//     // async_read(
//     //     socket_,
//     //     buffer(msg_.body.data(), kMaxNetMsgLen),
//     //     std::bind(
//     //         &Session::HandleReadHeader,
//     //         this, std::placeholders::_1
//     //     )
//     // );
//     std::cout << "受到连接请求\n";
//     async_read(
//         socket_,
//         // buffer(msg_.body.data(), msg_.Size()),
//         buffer(buf_.data(), buf_.size()),
//         std::bind(
//             &Session::HandleReadBody,
//             this, std::placeholders::_1
//         )
//     );
// }

// // void platform::Session::HandleReadHeader(const boost::system::error_code &error) {
// //     if (!error) {
// //         async_read(
// //             socket_,
// //             buffer(msg_.body.data(), .size()),
// //             std::bind(
// //                 &Session::HandleReadBody,
// //                 this, std::placeholders::_1
// //             )
// //         );
// //     }
// // }

// void platform::Session::HandleReadBody(const boost::system::error_code &error) {
//     if (!error) {
//         // async_read(
//         //     socket_,
//         //     buffer(buf_.data(), Session::kMsgHeaderLen),
//         //     std::bind(
//         //         &Session::HandleReadHeader,
//         //         this, std::placeholders::_1
//         //     )
//         // );
//         async_read(
//             socket_,
//             // buffer(msg_.body.data(), msg_.Size()),
//             buffer(buf_.data(), buf_.size()),
//             std::bind(
//                 &Session::HandleReadBody,
//                 this, std::placeholders::_1
//             )
//         );
//         for (auto c : buf_) {
//             std::cout << c;
//         }
//         std::cout << "\n";
//     }
// }

// platform::ServerOP::ServerOP(boost::asio::io_service &io_service,
//                              const boost::asio::ip::tcp::endpoint &endpoint,
//                              const std::string &server_id)
//     : io_service_(io_service),
//       acceptor_(io_service, endpoint),
//       port_(endpoint.port()),
//       server_id_(server_id),
//       thread_pool_() {
//     auto session_ptr = std::make_shared<Session>(io_service_);
//     acceptor_.async_accept(
//         session_ptr->socket(),
//         std::bind(
//             &ServerOP::HandleAccept, this,
//             session_ptr, std::placeholders::_1
//         )
//     );
// }

// void platform::ServerOP::HandleAccept(std::shared_ptr<Session> session_ptr,
//                                       const boost::system::error_code &error) {
//     if (!error) {
//         // 添加任务
//         post(thread_pool_, std::bind(&Session::Start, session_ptr));
//         // 继续等待连接
//         auto new_session_ptr = std::make_shared<Session>(io_service_);
//         acceptor_.async_accept(
//             new_session_ptr->socket(),
//             std::bind(
//                 &ServerOP::HandleAccept, this,
//                 new_session_ptr, std::placeholders::_1
//             )
//         );   
//     }
// }

// bool platform::Session::SeckeyAgree() {
//     return true;
// }

// bool platform::Session::SeckeyVerify() {

//     return true;
// }

// bool platform::Session::SeckeyLogout() {
    
//     return true;
// }