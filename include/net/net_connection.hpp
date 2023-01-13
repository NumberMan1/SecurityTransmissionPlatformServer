/*
参考https://github.com/OneLoneCoder/Javidx9/blob/master/PixelGameEngine/BiggerProjects/Networking
*/
#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include "net_msg.hpp"
#include "net_tsqueue.hpp"
namespace mine_net {

template<typename T>
class ServerInterface;

template<typename T>
class Connection : public std::enable_shared_from_this<Connection<T>> {
public:
    // 一个连接要么属于服务器，要么属于客户端
    // 两者的行为略有不同。
    enum class Owner {
        kServer,
        kClient
    };

    // 构造函数:指定所有者，连接到io_service，传输套接字
    // 提供传入消息队列的引用
    Connection(Owner parent, boost::asio::io_service &asioContext,
               boost::asio::ip::tcp::socket socket, TSQueue<OwnedMessage<T>> &qIn)
        : m_asioService(asioContext),
          m_socket(std::move(socket)),
          m_qMessagesIn(qIn) {
        m_nOwnerType = parent;
        if (m_nOwnerType == Owner::kServer) {
            // 连接是Server -> Client，为Client构造随机数据
            // 转换并发回验证
            m_nHandshakeOut = std::uint64_t(
                std::chrono::system_clock::now().time_since_epoch().count()
            );
            // 预先计算结果，以便在客户端响应时检查
            m_nHandshakeCheck = this->Scramble(m_nHandshakeOut);
        } else {
            // 连接是Client ->服务器，所以我们没有什么需要定义的
            m_nHandshakeIn = 0;
            m_nHandshakeOut = 0;
        }
    }

    virtual ~Connection() {}

    // 这个ID在系统范围内使用——客户端通过它来理解其他客户端
    // 存在于整个系统中
    uint32_t GetID() const {
        return id;
    }

    void ConnectToClient(ServerInterface<T> *server, uint32_t uid = 0) {
        if (m_nOwnerType == Owner::kServer) {
            if (m_socket.is_open()) {
                id = uid;
                // ReadHeader();

                // 客户端已经尝试连接到服务器，但我们希望
                // 客户端首先验证自己，因此首先将
                // 待验证的握手数据
                WriteValidation();
                // 接下来，执行一个任务，等待精确的异步执行
                // 客户端返回的验证数据
                ReadValidation(server);
            }
        }
    }

    void ConnectToServer(const boost::asio::ip::tcp::resolver::results_type& endpoints) {
        using namespace boost;
        // 只有客户端才能连接服务器
        if (m_nOwnerType == Owner::kClient) {
            // 请求asio尝试连接到一个端点
            asio::async_connect(m_socket, endpoints,
                [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
                    if (!ec) {
                        // ReadHeader();

                        // 服务器要做的第一件事是发送数据包以进行验证
                        // 所以等待并响应
                        ReadValidation();
                    }
                });
        }
    }


    void Disconnect() {
        using namespace boost;
        if (IsConnected())
            asio::post(m_asioService, [this]() { m_socket.close(); });
    }

    bool IsConnected() const {
        return m_socket.is_open();
    }

    // // 准备连接以等待传入的消息
    // void StartListening() {
        
    // }

    // 异步-发送消息，连接是一对一的，所以不需要指定
    // 对于客户端来说，目标就是服务器，反之亦然
    void Send(const Message<T>& msg) {
        using namespace boost;
        asio::post(m_asioService,
            [this, msg]() {
                // 如果队列中有消息，那么我们必须
                // 假设它正在异步编写的过程中
                // 两种方式都可以将消息添加到要输出的队列中如果没有消息
                // 可以被写入，然后开始写入进程
                // 队列最前面的消息
                bool bWritingMessage = !m_qMessagesOut.empty();
                m_qMessagesOut.push_back(msg);
                if (!bWritingMessage) {
                    WriteHeader();
                }
            });
    }


private:
    // ASYNC -用于写入消息头的主要上下文
    void WriteHeader() {
        using namespace boost;
        // 至少要发送一条消息所以分配一个传输缓冲区来保存
        // 消息，并发出work - asio，发送这些字节
        // 如果这个函数被调用，我们知道传出的消息队列一定有
        asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header,
            sizeof(MessageHeader<T>)),
            [this](std::error_code ec, std::size_t length) {
                // asio现在已经发送了字节——如果有问题的话
                // 错误是可用的……
                if (!ec) {
                    // ……没有错误，所以检查消息头是否刚刚发送
                    // 有一个消息主体…
                    if (m_qMessagesOut.front().body.size() > 0) {
                        //……如果有，所以执行这个任务来写入主体字节
                        WriteBody();
                    } else {
                        // ……它没有，所以我们结束了这个消息。把它从
                        // 传出消息队列
                        m_qMessagesOut.pop_front();

                        // 如果队列不为空，则有更多消息要发送，因此
                        // 通过启动任务来发送下一个header。
                        if (!m_qMessagesOut.empty()) {
                            WriteHeader();
                        }
                    }
                } else {
                    // ……asio没能写消息，我们可以分析原因
                    // 现在简单地假设连接已经关闭
                    // 套接字。当将来试图向该客户端写请求失败时
                    // 发送到关闭的socket，它将被清理。
                    std::cout << "[" << id << "] 写Header失败.\n";
                    m_socket.close();
                }
            });
    }

    // ASYNC -用于写入消息体的主要上下文
    void WriteBody() {
        using namespace boost;
        // 如果这个函数被调用，则表示刚刚发送了一个header，而这个header
        // 表示此消息的body已经存在填充传输缓冲区
        // 将body数据发送出去!
        asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(),
            m_qMessagesOut.front().body.size()),
            [this](std::error_code ec, std::size_t length) {
                if (!ec) {
                    // 发送成功，所以我们完成了消息
                    // 并将其从队列中删除
                    m_qMessagesOut.pop_front();

                    // 如果队列中仍然有消息，则发出任务
                    // 发送下一条消息的header
                    if (!m_qMessagesOut.empty()) {
                        WriteHeader();
                    }
                } else {
                    // 发送失败，参见对应的WriteHeader()的描述
                    std::cout << "[" << id << "] 写Body失败.\n";
                    m_socket.close();
                }
            });
    }

    // ASYNC -准备读取消息头的主要上下文
    void ReadHeader() {
        using namespace boost;
        // 如果这个函数被调用，我们希望asio一直等到它接收到
        // 足够构成消息头的字节数我们知道标题是固定的
        // 大小，因此分配一个足够大的传输缓冲区来存储它。
        // 我们将在一个“临时”Message对象中构建消息方便使用
        asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header,
            sizeof(MessageHeader<T>)),
            [this](std::error_code ec, std::size_t length) {						
                if (!ec) {
                    // 已读取完整的消息头，请检查此消息是否已读取
                    // 后面有一个主体…
                    if (m_msgTemporaryIn.header.size > 0) {
                        // ……有的，所以在消息体中分配足够的空间
                        // vector，并调用asio来读取函数体。
                        m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
                        ReadBody();
                    } else {
                        // 没有，所以将这条body消息添加到连接中
                        // 传入消息队列
                        AddToIncomingMessageQueue();
                    }
                } else {
                    // 从客户端读取出错，很可能是断开连接
                    // 已经发生。关闭套接字，让系统稍后整理。
                    std::cout << "[" << id << "] 读Header失败.\n";
                    m_socket.close();
                }
            });
    }

    // ASYNC -准备读取消息体的主要上下文
    void ReadBody() {
        using namespace boost;
        // 如果这个函数被调用，说明已经读取了一个Header，并且这个Header
        // 请求读取body, body的空间已经被分配
        // 在临时消息对象中，所以只需要等待字节到达……
        asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(),
            m_msgTemporaryIn.body.size()),
            [this](std::error_code ec, std::size_t length) {						
                if (!ec) {
                    // ……他们做到了!消息现在完成了，所以添加
                    // 将整个消息发送到传入队列
                    AddToIncomingMessageQueue();
                } else {
                    // 如上所述
                    std::cout << "[" << id << "] 读Body失败.\n";
                    m_socket.close();
                }
            });
    }

    // “加密”数据
    std::uint64_t Scramble(std::uint64_t nInput) {
        std::uint64_t out = nInput ^ 0xDEADBEEFC0DECAFE;
        out = (out& 0xF0F0F0F0F0F0F0) >> 4 | (out & 0x0F0F0F0F0F0F0F) << 4;
        return out ^ 0xC0DEFACE12345678;
    }

    // ASYNC -客户端和服务器都使用它来编写验证包
    void WriteValidation() {
        using namespace boost;
        asio::async_write(m_socket, asio::buffer(&m_nHandshakeOut, sizeof(uint64_t)),
            [this](std::error_code ec, std::size_t length) {
                if (!ec) {
                    // 验证数据发送后，客户端应该等待获取响应(或闭包)
                    if (m_nOwnerType == Owner::kClient)
                        ReadHeader();
                } else {
                    m_socket.close();
                }
            });
    }

    void ReadValidation(ServerInterface<T>* server = nullptr) {
        using namespace boost;
        asio::async_read(m_socket, asio::buffer(&m_nHandshakeIn, sizeof(uint64_t)),
            [this, server](std::error_code ec, std::size_t length) {
                if (!ec) {
                    if (m_nOwnerType == Owner::kServer) {
                        // 连接是一个服务器，所以检查来自客户端的响应

                        // 将发送的数据与实际解决方案进行比较
                        if (m_nHandshakeIn == m_nHandshakeCheck) {
                            // 客户端已经提供了有效的解决方案，所以允许它正常连接
                            std::cout << "客户端验证" << std::endl;
                            server->OnClientValidated(this->shared_from_this());

                            // 现在等待接收数据
                            ReadHeader();
                        } else {
                            // 客户端提供了错误的数据，因此断开连接
                            std::cout << "客户端断开(验证失败)" << std::endl;
                            m_socket.close();
                        }
                    } else {
                        // 连接是一个客户端，所以解开谜题
                        m_nHandshakeOut = this->Scramble(m_nHandshakeIn);

                        // 写入结果
                        WriteValidation();
                    }
                } else {
                    //发生了一些故障
                    std::cout << "客户端断开(重新验证)" << std::endl;
                    m_socket.close();
                }
            });
	}

    // 收到完整消息后，将其添加到传入队列
    void AddToIncomingMessageQueue() {				
        // 通过初始化将其插入队列，将其转换为“自有消息”
        // 使用来自此连接对象的共享指针
        if(m_nOwnerType == Owner::kServer)
            m_qMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });
        else
            m_qMessagesIn.push_back({ nullptr, m_msgTemporaryIn });

        // 我们现在必须准备asio io_service以接收下一条消息。它
        // 将静坐等待字节到达，并等待消息构造
        // 进程会重复自身
        ReadHeader();
    }

protected:
    // 每个连接都有一个唯一的远程连接套接字
    boost::asio::ip::tcp::socket m_socket;

    // 该io_service与整个asio实例共享
    boost::asio::io_service& m_asioService;

    // 这个队列保存了所有要发送到远程端的消息
    TSQueue<Message<T>> m_qMessagesOut;

    // This引用父对象的传入队列
    TSQueue<OwnedMessage<T>>& m_qMessagesIn;

    // 传入的消息是异步构建的，所以我们会
    // 在这里存储组装好的消息，直到它准备好
    Message<T> m_msgTemporaryIn;

    //“所有者”决定连接的某些行为
    Owner m_nOwnerType = Owner::kServer;
    // 握手验证
    std::uint64_t m_nHandshakeOut = 0;
    std::uint64_t m_nHandshakeIn = 0;
    std::uint64_t m_nHandshakeCheck = 0;
	
    bool m_bValidHandshake = false;
	bool m_bConnectionEstablished = false;
    
    std::uint32_t id = 0;

};

}