#ifndef NODE_SECKEY_H
#define NODE_SECKEY_H

#include <array>

namespace transmission_msg {

struct NodeSeckey {
    int status{0};      // 秘钥状态: 1可用, 0:不可用
    int seckeyID{0};    // 秘钥的编号
    std::array<char, 12> clientID{0};    // 客户端ID, 客户端的标识
    std::array<char, 12> serverID{0};    // 服务器ID, 服务器标识
    std::array<char, 128> seckey{0};	// 对称加密的秘钥
    bool operator==(const NodeSeckey &other) const noexcept {
        return this->seckeyID == other.seckeyID;
    }
};

} // namespace transmission_msg

#endif