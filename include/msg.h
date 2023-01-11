#ifndef MSG_H
#define MSG_H

#include <string>
#include <memory>
#include <vector>
#include <cstring>
#include <type_traits>
#include "Proto/Message.pb.h"

namespace transmission_msg {

namespace net {

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
                      "data的类型对于提取到vector过于复杂");
        std::size_t size_later = msg.body.size() - sizeof(DataType);
        std::memcpy(&data, msg.body.data() + size_later, sizeof(DataType));
        msg.body.resize(size_later);
        msg.header.size = msg.Size();
        return msg;
    }
};

} // namespace transmission_msg::net

namespace proto {

struct Info {
	std::string client_id;
	std::string server_id;
	std::string data;
};

class Msg {
public:
    explicit Msg() = default;
    virtual ~Msg() = default;	
    inline virtual std::string EncodeMsg() const {
        return std::string {};
    }
    inline std::shared_ptr<Info> DecodeMsg() {
        return nullptr; 
    }
};

struct RequestInfo
    : public Info
{
	proto_info::CmdType cmd;
	std::string sign;
};

class RequestMsg 
    : public Msg
{
public:
	// 空对象
	explicit RequestMsg() = default;
	// 构造出的对象用于 解码 的场景
	explicit RequestMsg(const std::string_view &enc_str)
        : Msg{} {
        InitMessage(enc_str);
    }
	// 构造出的对象用于 编码 场景
	explicit RequestMsg(const RequestInfo *info) 
        : Msg{} {
        InitMessage(info);
    }
	virtual ~RequestMsg() = default;
	// Init函数给空构造准备 的s
	// 解码使用
	inline void InitMessage(const std::string_view &enc_str) {
        enc_str_ = enc_str;
    }
	// 编码时候使用
	inline void InitMessage(const RequestInfo *info) {
    	msg_.set_cmd_type(info->cmd);
	    msg_.set_client_id(info->client_id);
	    msg_.set_server_id(info->server_id);
	    msg_.set_sign(info->sign);
	    msg_.set_data(info->data);
    }
	// 重写的父类函数 -> 序列化函数, 返回序列化的字符串
	inline virtual std::string EncodeMsg() const override {
        std::string output(msg_.SerializeAsString());
        return output;
    }
	// 重写的父类函数 -> 反序列化函数, 返回结构体/类对象
    inline std::shared_ptr<RequestInfo> DecodeMsg() {
        msg_.ParseFromString(enc_str_);
        return std::make_shared<RequestInfo>(
        RequestInfo{
            Info {
                msg_.client_id(),
                msg_.server_id(),
                msg_.data()
            },
            msg_.cmd_type(),
            msg_.sign(),
        });
    }

private:
    std::string enc_str_;
    proto_info::RequestMsg msg_;
};

struct RespondInfo
    : public Info
{
	std::int32_t status;
	std::int32_t seckey_id;
};

class RespondMsg 
    : public Msg
{
public:
	// 空对象
	RespondMsg() = default;
    // 构造出的对象用于 解码 的场景
	RespondMsg(const std::string_view &enc)
        : Msg{} {
        InitMessage(enc);
    }
	// 构造出的对象用于 编码 场景
	RespondMsg(const RespondInfo *info) 
        : Msg{} {
        InitMessage(info);
    }
	virtual ~RespondMsg() = default;
	// Init函数给空构造准备 的s
	// 解码使用
	inline void InitMessage(const std::string_view &enc) {
        enc_str_ = enc;
    }
	// 编码时候使用
	inline void InitMessage(const RespondInfo *info) {
        msg_.set_status(info->status);
        msg_.set_seckey_id(info->seckey_id);
        msg_.set_client_id(info->client_id);
        msg_.set_server_id(info->server_id);
        msg_.set_data(info->data);
    }
	// 重写的父类函数 -> 序列化函数, 返回序列化的字符串
	inline virtual std::string EncodeMsg() const override {
        std::string output(msg_.SerializeAsString());
        return output;
    }
 	// 重写的父类函数 -> 反序列化函数, 返回结构体/类对象
	inline std::shared_ptr<RespondInfo> DecodeMsg() {
        msg_.ParseFromString(enc_str_);
        return std::make_shared<RespondInfo>(
        RespondInfo{
            Info {
                msg_.client_id(),
                msg_.server_id(),
                msg_.data()
            },
            msg_.status(),
            msg_.seckey_id()
        });
    }

private:
	std::string enc_str_;
	proto_info::RespondMsg msg_;
};


} // namespace transmission_msg::proto

} // namespace transmission_msg

#endif // MSG_H
