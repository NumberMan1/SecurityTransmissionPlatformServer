/*
参考https://github.com/OneLoneCoder/Javidx9/blob/master/PixelGameEngine/BiggerProjects/Networking
*/
#pragma once

#include "net_connection.hpp"
namespace mine_net {

template<typename T>
class ClientInterface {
public:
    ClientInterface() { }

    virtual ~ClientInterface() {
        // 如果Client被销毁，总是尝试断开与服务器的连接
        Disconnect();
    }

public:
    // 使用主机名/ip地址和端口连接到服务器
    bool Connect(const std::string& host, const std::uint16_t port) {
        using namespace boost;
        try {
            // 将主机名/ip-address解析为实际的物理地址
            asio::ip::tcp::resolver resolver(m_service);
            asio::ip::tcp::resolver::results_type endpoints = 
                resolver.resolve(host, std::to_string(port));

            // 创建连接
            m_connection = std::make_unique<Connection<T>>(
                    Connection<T>::Owner::kClient,
                    m_service,
                    asio::ip::tcp::socket(m_service),
                    m_qMessagesIn
                );

            // 连接对象连接到服务器
            m_connection->ConnectToServer(endpoints);

            // 启动io_service线程
            thrIOService = std::thread([this]() { m_service.run(); });
        } catch (std::exception& e) {
            std::cerr << "客户端异常: " << e.what() << "\n";
            return false;
        }
        return true;
    }

    // 与服务器断开连接
    void Disconnect() {
        // 如果连接存在，并且已经连接，那么……
        if(IsConnected()) {
            // …正常断开与服务器的连接
            m_connection->Disconnect();
        }

        // 无论如何，我们也完成了asio io_service…		
        m_service.stop();
        if (thrIOService.joinable())
            thrIOService.join();

        // 销毁连接对象
        auto useless_connection_ptr = m_connection.release();
        if (useless_connection_ptr)
            delete useless_connection_ptr;
    }

    // 检查Client是否实际连接到服务器
    bool IsConnected() {
        if (m_connection)
            return m_connection->IsConnected();
        else
            return false;
    }

public:
    // 向服务器发送消息
    void Send(const Message<T>& msg) {
        if (IsConnected())
            m_connection->Send(msg);
    }

    // 检索服务器发来的消息队列
    TSQueue<OwnedMessage<T>>& Incoming() { 
        return m_qMessagesIn;
    }

protected:
    // asio io_service处理数据传输…
    boost::asio::io_service m_service;
    // …但是需要一个自己的线程来执行它的工作命令
    std::thread thrIOService;
    // Client有一个“Connection”对象的实例，用于处理数据传输
    std::unique_ptr<Connection<T>> m_connection = nullptr;
    
private:
    // 这是来自服务器的传入消息的线程安全队列
    TSQueue<OwnedMessage<T>> m_qMessagesIn;
};

template<typename T>
class ServerInterface
{
public:
    // 创建一个服务器，准备监听指定的端口
    ServerInterface(std::uint16_t port)
        : m_asioAcceptor(
                m_asioService,
                boost::asio::ip::tcp::endpoint(
                        boost::asio::ip::tcp::v4(), port
                )
            ) { }

    virtual ~ServerInterface() {
        Stop();
    }

    // 启动服务器!
    bool Start() {
        try {
            // 将任务发送到asio io_service——这很重要
            // 因为它会用"work"启动io_service，然后停止它
            // 立即退出。因为这是一个服务器，我们
            // 希望它就绪以处理客户端尝试连接。
            WaitForClientConnection();

            // 在自己的线程中启动asio io_service
            m_threadIOService = std::thread([this]() { m_asioService.run(); });
        } catch (std::exception& e) {
            // 某些东西禁止服务器监听
            std::cerr << "[SERVER] 异常: " << e.what() << "\n";
            return false;
        }

        std::cout << "[SERVER] 启动!\n";
        return true;
    }

    // 停止服务器!
    void Stop() {
        // 请求关闭io_service
        m_asioService.stop();

        // 整理io_service线程
        if (m_threadIOService.joinable()) m_threadIOService.join();

        std::cout << "[SERVER] 停止!\n";
    }

    // ASYNC -指示asio等待连接
    void WaitForClientConnection() {
        using namespace boost;
        // 每次传入的连接尝试
        m_asioAcceptor.async_accept(
            [this](std::error_code ec, asio::ip::tcp::socket socket) {
                // 由传入的连接请求触发
                if (!ec) {
                    std::cout << "[SERVER] 新的连接: " << socket.remote_endpoint() << "\n";

                    // 创建一个新连接来处理这个客户端
                    std::shared_ptr<Connection<T>> newconn = 
                        std::make_shared<Connection<T>>(Connection<T>::Owner::kServer, 
                            m_asioService, std::move(socket), m_qMessagesIn);

                    // 给用户服务器一个拒绝连接的机会
                    if (OnClientConnect(newconn)) {								
                        // 允许连接，所以添加到新连接容器
                        m_deqConnections.push_back(std::move(newconn));

                        // 向连接发出一个任务用来等待字节到达!
                        m_deqConnections.back()->ConnectToClient(this, nIDCounter++);

                        std::cout << "[" << m_deqConnections.back()->GetID() << "] 连接批准\n";
                    } else {
                        std::cout << "[-----] 连接否认\n";

                        // 连接将超出范围，没有挂起的任务，也会
                        // 由于智能指针，会自动销毁
                    }
                } else {
                    // 验收过程中发生错误
                    std::cout << "[SERVER] 新连接错误: " << ec.message() << "\n";
                }

                // 用更多的工作准备asio io_service——同样只是等待另一个连接…
                WaitForClientConnection();
            });
    }

    // 向特定的客户端发送消息
    void MessageClient(std::shared_ptr<Connection<T>> client,
                       const Message<T>& msg) {
        // 检查客户端是合法的…
        if (client && client->IsConnected()) {
            // …并通过连接发布消息
            client->Send(msg);
        } else {
            // 如果我们不能与客户端沟通，我们可以
            // 删除客户端——让服务器知道，它可以这么做
            // 以某种方式跟踪它
            OnClientDisconnect(client);
            client.reset();

            // 然后将其从容器中移除
            m_deqConnections.erase(
                std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections.end());
        }
    }
    
    // 向所有客户端发送消息
    void MessageAllClients(const Message<T>& msg,
            std::shared_ptr<Connection<T>> pIgnoreClient = nullptr) {
        bool bInvalidClientExists = false;

        for (auto& client : m_deqConnections) {
            // 检查客户端是否已连接…
            if (client && client->IsConnected()) {
                if(client != pIgnoreClient)
                    client->Send(msg);
            } else {
                // 客户端联系不上，那么就断开连接。
                OnClientDisconnect(client);
                client.reset();

                // 设置此标志，然后从容器中移除无效客户端
                bInvalidClientExists = true;
            }
        }

        // 移除无效客户端，一次性完成
        // 这样，我们就不会让无效客户端在容器中迭代
        if (bInvalidClientExists)
            m_deqConnections.erase(
                std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
    }

    // 强制服务器响应传入的消息
    void Update(size_t nMaxMessages = -1, bool bWait = false) {
        if (bWait) m_qMessagesIn.wait();

        // 根据指定的值处理尽可能多的消息
        size_t nMessageCount = 0;
        while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty()) {
            auto msg = m_qMessagesIn.pop_front();

            // 传递给消息处理程序
            OnMessage(msg.remote, msg.msg);

            nMessageCount++;
        }
    }

protected:
    // 这个server类应该覆盖这些函数来实现
    // 自定义功能

    // 当客户端连接时调用，可以通过返回false否决连接
    virtual bool OnClientConnect(std::shared_ptr<Connection<T>> client)  {
        return false;
    }

    // 当客户端似乎已断开连接时调用
    virtual void OnClientDisconnect(std::shared_ptr<Connection<T>> client)  {

    }

    // 收到消息时调用
    virtual void OnMessage(std::shared_ptr<Connection<T>> client, Message<T>& msg)  {

    }

public:
    // 在验证客户端时调用
    virtual void OnClientValidated(std::shared_ptr<Connection<T>> client)  {

    }


protected:
    // 传入消息包的线程安全队列
    TSQueue<OwnedMessage<T>> m_qMessagesIn;

    // 活动验证连接的容器
    std::deque<std::shared_ptr<Connection<T>>> m_deqConnections;

    // 声明的顺序很重要——它也是初始化的顺序
    boost::asio::io_service m_asioService;
    std::thread m_threadIOService;

    // 处理新的连接尝试…
    boost::asio::ip::tcp::acceptor m_asioAcceptor;

    // 客户将通过ID在“更广泛的系统”中被识别
    std::uint32_t nIDCounter = 10000;
};

}