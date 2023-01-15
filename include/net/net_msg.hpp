/*
参考https://github.com/OneLoneCoder/Javidx9/blob/master/PixelGameEngine/BiggerProjects/Networking
*/
#pragma once
#include <type_traits>
#include <vector>
#include <cstring>
#include <memory>

namespace mine_net {

template<typename T>
struct MessageHeader {
    T id{};
    std::uint32_t size = 0;
};

template<typename T>
struct Message {
    MessageHeader<T> header{};
    std::vector<std::uint8_t> body;
    // 以字节为单位返回整个消息包主体的大小
    inline std::size_t Size() const {
        return body.size();
    }
    // 重写std::cout兼容性——生成友好的消息描述
    friend std::ostream& operator<<(std::ostream &os,
                                    const Message<T> &msg) {
        os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
        return os;
    }
    /*
    便利运算符重载——允许我们添加和删除内容
    把body向量当成一个栈，所以是先入后出的。这些是
    模板本身，因为我们不知道用户推送的数据类型
    弹出，所以让我们允许它们全部弹出。注意:它假设数据类型基本上是
    普通旧数据(POD)。TLDR:序列化和反序列化到向量/从向量
    将任何类似pod的数据推入Message
    */
    template<typename DataType>
    friend Message<T>& operator<<(Message<T> &msg,
                                  const DataType &data) {
        // 检查类型是否可用
        static_assert(std::is_standard_layout_v<DataType>,
                      "data的类型对于推入到vector过于复杂");
        std::size_t size_former = msg.body.size();
        msg.body.resize(msg.body.size() + sizeof(DataType));
        std::memcpy(msg.body.data() + size_former, &data, sizeof(DataType));
        msg.header.size = msg.Size();
        return msg;
    }
    // 将POD类型的数据从Message中提取出来
    template<typename DataType>
    friend Message<T>& operator>>(Message<T> &msg,
                                  DataType &data) {
        // 检查类型是否可用
        static_assert(std::is_standard_layout_v<DataType>,
                      "data的类型对于提取出vector过于复杂");
        std::size_t size_later = msg.body.size() - sizeof(DataType);
        std::memcpy(&data, msg.body.data() + size_later, sizeof(DataType));
        msg.body.resize(size_later);
        msg.header.size = msg.Size();
        return msg;
    }
};
/*
“拥有的”消息与普通消息相同，但它与
一个连接。在服务器上，所有者是发送消息的客户端，
在客户端，所有者就是服务器。
*/
template <typename T>
class Connection;

template <typename T>
struct OwnedMessage {
    std::shared_ptr<Connection<T>> remote = nullptr;
    Message<T> msg;

    friend std::ostream& operator<<(std::ostream& os,
                                    const OwnedMessage<T>& msg) {
        os << msg.msg;
        return os;
    }
};
}