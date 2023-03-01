#ifndef STRUCT_SECKEY_H
#define STRUCT_SECKEY_H

#include <string>
#include <array>
#include <queue>

namespace platform {

// 形式为 xxxx-xx-xx
using Date = std::array<char, 11>;

constexpr std::size_t kKeyIdLen{9};
constexpr std::size_t kNodeIdLen{4};
constexpr std::size_t kAuthCodeLen{12};
constexpr std::size_t kAESBlockLen{128};

struct DBInfo {
    std::string host_name;
    std::string user_name;
    std::string password;
    std::string data_base;
};

enum class TableName {
    kSeckey_info,
    kSeckey_node,
};

struct RowSeckeyInfo {
    std::array<char, kKeyIdLen + 1> key_id;
    Date create_time;
    bool state{false};
    std::array<char, kNodeIdLen + 1> client_id,
                        server_id;
    std::string seckey; // 最多不超过512个字符(不包含\0)
};

struct RowSeckeyNode {
    std::array<char, kNodeIdLen + 1> id;
    std::string name; // 最多不超过128个字符(不包含\0)
    Date create_time;
    std::array<char, kAuthCodeLen + 1> authcode; // 授权码
    bool state{false};
    std::string node_desc; // 描述, 最多不超过512个字符(不包含\0)
};

struct ShmNodeSeckey {
    bool status{false};      // 秘钥状态
    std::uint32_t seckey_id{0};    // 秘钥的编号
    std::array<char, 12> client_id{0};    // 客户端ID, 客户端的标识
    std::array<char, 12> server_id{0};    // 服务器ID, 服务器标识
    std::array<char, kAESBlockLen + 1> seckey{0};	// 对称加密的秘钥
    bool operator==(const ShmNodeSeckey &other) const noexcept {
        return this->seckey_id == other.seckey_id;
    }
};

} // namespace transmission_msg

#endif