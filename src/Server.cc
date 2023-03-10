#include "Server.h"
#include <iostream>
#include <fstream>
#include <random>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "openssl/mine_hash.hpp"
#include "seckey_shm.hpp"

inline namespace {

using TInfo = transmission_msg::proto::Info;
using TRTInfo = transmission_msg::proto::RequestInfo;
using TRPInfo = transmission_msg::proto::RespondInfo;
using TFactory = transmission_msg::proto::factory::Factory;
using TRTFactory = transmission_msg::proto::factory::RequestFactory;
using TRPFactory = transmission_msg::proto::factory::RespondFactory;

constexpr std::size_t kSeckeyLen = 128;

struct SeckeyInfo {
    std::string cline_id;
    std::string server_id;
    std::array<char, kSeckeyLen> seckey;
    bool status;
};

} // namespace ::

platform::Server::Server(std::string server_id, std::uint16_t port)
    : ServerInterface(port),
      server_id_(server_id) {

}

// bool platform::Server::CheckSign(const std::string_view &pubkey_file_name,
//                                  const std::string_view &data,
//                                  const std::string_view &sign_data) {
//     mine_openssl::MyRSA rsa(pubkey_file_name);
//     return rsa.Verify(data, sign_data);
// }

auto platform::Server::ParseMsgImpl(mine_net::Message<TMsgType> &msg) {
    // θΏθ‘ζε
    std::size_t net_msg_len = msg.Size();
    std::string net_str;
    for (std::size_t i = 0; i != net_msg_len; ++i) {
        char c;
        msg >> c;
        net_str += c;
    }
    TRTFactory requset_factory {net_str};
    auto request_msg = requset_factory.CreateMsg();
    auto request_info = request_msg->DecodeMsg();    
    std::string client_id_pubkey_file_name = request_info->client_id;
    client_id_pubkey_file_name += "_pubkey.pem";
    client_id_pubkey_file_name.insert(0, "seckey/", 0, std::strlen("seckey/"));
    return std::make_tuple(request_info, client_id_pubkey_file_name);
}

bool platform::Server::SeckeyAgree
    (std::shared_ptr<mine_net::Connection<TMsgType>> client,
     mine_net::Message<TMsgType> &msg) {
    // θΏθ‘ζε
    // std::size_t net_msg_len = msg.Size();
    // std::string net_str;
    // for (std::size_t i = 0; i != net_msg_len; ++i) {
    //     char c;
    //     msg >> c;
    //     net_str += c;
    // }
    // using TInfo = transmission_msg::proto::Info;
    // using TRTInfo = transmission_msg::proto::RequestInfo;
    // using TRPInfo = transmission_msg::proto::RespondInfo;
    // using TFactory = transmission_msg::proto::factory::Factory;
    // using TRTFactory = transmission_msg::proto::factory::RequestFactory;
    // using TRPFactory = transmission_msg::proto::factory::RespondFactory;
    auto request_tuple = ParseMsgImpl(msg);
    auto request_info = std::get<0>(request_tuple);
    auto client_id_pubkey_file_name = std::get<1>(request_tuple);
    // TRTFactory requset_factory {net_str};
    // auto request_msg = requset_factory.CreateMsg();
    // auto request_info = request_msg->DecodeMsg();
    // std::string client_id_pubkey_file_name = request_info->client_id;
    // client_id_pubkey_file_name += "_pubkey.pem";
    // client_id_pubkey_file_name.insert(0, "seckey/", 0, std::strlen("seckey/"));
    // εε₯η£η
    if (std::filesystem::exists(client_id_pubkey_file_name)) {
        spdlog::warn("η­Ύεζ ‘ιͺε€±θ΄₯");
        auto logger = spdlog::basic_logger_mt("SeckeyAgree logger", "log/server");
        logger->warn("η­Ύεζ ‘ιͺε€±θ΄₯, ε·²η»ζεεε―ι₯");
        return false;
    }
    std::ofstream file_out(client_id_pubkey_file_name);
    file_out << request_info->data;
    file_out.close();
    // ιͺθ―η­Ύε
    mine_openssl::Hash h(mine_openssl::HashType::kSHA384Type);
    h.Update(request_info->data);
    if (!CheckSign(client_id_pubkey_file_name,
            h.Final(), request_info->sign)) {
        spdlog::warn("η­Ύεζ ‘ιͺε€±θ΄₯");
        auto logger = spdlog::basic_logger_mt("SeckeyAgree logger", "log/server");
        logger->warn("η­Ύεζ ‘ιͺε€±θ΄₯");
        std::filesystem::remove(client_id_pubkey_file_name);
        return false;
    }
    // εε€δΌ θΎζ°ζ?
    mine_net::Message<TMsgType> msg_out;
    msg_out.header.id = TMsgType::kSeckeyAgree;
    // ηζιζΊε­η¬¦δΈ², ε―Ήη§°ε ε―ηε―ι₯
    std::string aes_key = GetAESRandStr(AESKeyLen::kLen16);
    // εε₯ε±δΊ«εε­



    mine_openssl::MyRSA rsa(client_id_pubkey_file_name);
    // ε¬ι₯ε ε―
    std::string aes_seckey = rsa.EncryptPubKey(aes_key);
    // εε€ζ°ζ?
    TRPInfo respond_info {
        TInfo {
            request_info->client_id,
            server_id_,
            aes_seckey
        },
        12 // ιθ¦ζ°ζ?εΊζδ½
    };
    TRPFactory respond_factory {&respond_info};
    auto respond_msg = respond_factory.CreateMsg();
    std::string respond_str(respond_msg->EncodeMsg());
    // η±δΊmsgζ―η±»δΌΌδΊstackεε₯εεΊοΌιθ¦εεεε­ε₯ζ°ζ?
    for (auto i = std::ssize(respond_str) - 1; i != -1; --i) {
        msg_out << respond_str.at(i);
    }
    std::cout << "aes : " << aes_key << "\n";
    // std::cout << respond_str << "\n";
    // ει
    client->Send(msg_out);
    return true;
}

bool platform::Server::SeckeyVerify
    (std::shared_ptr<mine_net::Connection<TMsgType>> client,
     mine_net::Message<TMsgType> &msg) {
    auto request_tuple = ParseMsgImpl(msg);
    auto request_info = std::get<0>(request_tuple);
    auto client_id_pubkey_file_name = std::get<1>(request_tuple);
    return true;
}

bool platform::Server::SeckeyLogout
    (std::shared_ptr<mine_net::Connection<TMsgType>> client,
     mine_net::Message<TMsgType> &msg) {
    auto request_tuple = ParseMsgImpl(msg);
    auto request_info = std::get<0>(request_tuple);
    auto client_id_pubkey_file_name = std::get<1>(request_tuple);
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

std::string platform::Server::GetAESRandStr(AESKeyLen len) {
    // δ»₯ε½εζΆι΄δΈΊη§ε­
    std::default_random_engine random_engine(std::time(nullptr));
    std::uniform_int_distribution<unsigned> unsigned_distribution;
    std::string retStr;
    std::string_view buf = "~`@#$%^&*()_+=-{}[];':"; // ηΉζ?ε­η¬¦ι
    for (std::int8_t i = 0; i < len; ++i) {
        std::int8_t flag = std::rand() % 4;
        switch (flag) {
        case 0:	// 0-9
            retStr.append(1, unsigned_distribution(random_engine) % 10 + '0');
            break;
        case 1:	// a-z
            retStr.append(1, unsigned_distribution(random_engine) % 26 + 'a');
            break;
        case 2:	// A-Z
            retStr.append(1, unsigned_distribution(random_engine) % 26 + 'A');
            break;
        case 3:	// ηΉζ?ε­η¬¦
            retStr.append(1, buf[unsigned_distribution(random_engine) % buf.size()]);
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
//     std::cout << "εε°θΏζ₯θ―·ζ±\n";
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
//         // ζ·»ε δ»»ε‘
//         post(thread_pool_, std::bind(&Session::Start, session_ptr));
//         // η»§η»­η­εΎθΏζ₯
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