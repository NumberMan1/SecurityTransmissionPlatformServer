#ifndef MSG_H
#define MSG_H

#include <string>
#include <memory>
#include "Proto/Message.pb.h"

namespace transmission_msg {

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
