#include "Server.h"
#include <iostream>
#include <fstream>

#include "openssl/mine_rsa.h"

bool platform::Server::CheckSign(const std::string_view &pubkey_file_name,
                                 const std::string_view &data,
                                 const std::string_view &sign_data) {
    mine_openssl::MyRSA rsa(pubkey_file_name);
    bool flag = rsa.Verify(data, sign_data);
    return flag;
}

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
    using TInfo = transmission_msg::proto::Info;
    using TRTInfo = transmission_msg::proto::RequestInfo;
    using TRPInfo = transmission_msg::proto::RespondInfo;
    using TFactory = transmission_msg::proto::factory::Factory;
    using TRTFactory = transmission_msg::proto::factory::RequestFactory;
    using TRPFactory = transmission_msg::proto::factory::RespondFactory;
    // 由于Message是后入先出，需要修正消息
    std::reverse(net_str.begin(), net_str.end());
    std::cout << net_str << '\n';
    TRTFactory requset_factory {net_str};
    auto request_msg = requset_factory.CreateMsg();
    auto request_info = request_msg->DecodeMsg();
    std::string client_id_pubkey_file_name = request_info->client_id;
    client_id_pubkey_file_name += "_pubkey.pem";
    // 写入磁盘
    std::ofstream file_out(client_id_pubkey_file_name);
    file_out << request_info->data;
    // 验证签名
    if (!CheckSign(client_id_pubkey_file_name,
            request_info->data, request_info->sign)) {
        std::filesystem::remove(client_id_pubkey_file_name);
        return false;
    }
    // 准备传输数据
    mine_net::Message<TMsgType> msg_out;
    msg_out.header.id = TMsgType::kSeckeyAgree;

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