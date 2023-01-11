#include "Server.h"
#include <iostream>
#include <thread>
using namespace boost::asio;

platform::Session::Session(boost::asio::io_service &io_service)
    : socket_(io_service) {
    
}

void platform::Session::Start() {
    // async_read(
    //     socket_,
    //     buffer(msg_.body.data(), kMaxNetMsgLen),
    //     std::bind(
    //         &Session::HandleReadHeader,
    //         this, std::placeholders::_1
    //     )
    // );
    async_read(
        socket_,
        buffer(msg_.body.data(), msg_.Size()),
        std::bind(
            &Session::HandleReadBody,
            this, std::placeholders::_1
        )
    );
}

// void platform::Session::HandleReadHeader(const boost::system::error_code &error) {
//     if (!error) {
//         async_read(
//             socket_,
//             buffer(msg_.body.data(), .size()),
//             std::bind(
//                 &Session::HandleReadBody,
//                 this, std::placeholders::_1
//             )
//         );
//     }
// }

void platform::Session::HandleReadBody(const boost::system::error_code &error) {
    if (!error) {
        // async_read(
        //     socket_,
        //     buffer(buf_.data(), Session::kMsgHeaderLen),
        //     std::bind(
        //         &Session::HandleReadHeader,
        //         this, std::placeholders::_1
        //     )
        // );
        async_read(
            socket_,
            buffer(msg_.body.data(), msg_.Size()),
            std::bind(
                &Session::HandleReadBody,
                this, std::placeholders::_1
            )
        );
    }
}

platform::ServerOP::ServerOP(boost::asio::io_service &io_service,
                             const boost::asio::ip::tcp::endpoint &endpoint,
                             const std::string &server_id)
    : io_service_(io_service),
      acceptor_(io_service, endpoint),
      port_(endpoint.port()),
      server_id_(server_id),
      thread_pool_() {
    auto session_ptr = std::make_shared<Session>(io_service_);
    acceptor_.async_accept(
        session_ptr->socket(),
        std::bind(
            &ServerOP::HandleAccept, this,
            session_ptr, std::placeholders::_1
        )
    );
}

void platform::ServerOP::HandleAccept(std::shared_ptr<Session> session_ptr,
                                      const boost::system::error_code &error) {
    if (!error) {
        // 添加任务
        post(thread_pool_, std::bind(&Session::Start, session_ptr));
        // 继续等待连接
        auto new_session_ptr = std::make_shared<Session>(io_service_);
        acceptor_.async_accept(
            new_session_ptr->socket(),
            std::bind(
                &ServerOP::HandleAccept, this,
                new_session_ptr, std::placeholders::_1
            )
        );   
    }
}

bool platform::ServerOP::SeckeyAgree() {
    return true;
}

bool platform::ServerOP::SeckeyVerify() {

    return true;
}

bool platform::ServerOP::SeckeyLogout() {
    
    return true;
}