#include "Server.h"
#include <iostream>
#include <thread>
using namespace boost::asio;

platform::Session::Session(boost::asio::io_service &io_service)
    : socket_(io_service) {
    
}

void platform::Session::Start() {
    std::cout << "hello" << '\n';
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
        auto session_ptr = std::make_shared<Session>(io_service_);
        post(thread_pool_, std::bind(&Session::Start, session_ptr.get()));
        acceptor_.async_accept(
            session_ptr->socket(),
            std::bind(
                &ServerOP::HandleAccept, this,
                session_ptr, std::placeholders::_1
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