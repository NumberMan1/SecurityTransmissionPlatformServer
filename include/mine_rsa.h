#ifndef MINE_RSA_H
#define MINE_RSA_H

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <memory>

namespace mine_openssl {

class MyRSA {
public:
    static constexpr int kKeyBitsLen = 1024;
    static constexpr std::size_t kBigNumW = 12345;
    // 用于生成n * 1024的bits的密钥
    explicit MyRSA(const int &n);
    // 用于初始化以拥有的密钥
    constexpr explicit MyRSA(const std::string_view &pub_key_path,
                             const std::string_view &pri_key_path)
        : _impl_(RSAImpl()) {
        Init(pub_key_path, pri_key_path);
    }
    ~MyRSA() = default;
    // 生成密钥对
    void Init(const std::string_view &pub_key_path,
              const std::string_view &pri_key_path);
    // 储存密钥对
    void SaveRSAKey(const std::string_view &pub_key_path,
                    const std::string_view &pri_key_path) const;
    // 公钥加密, 每次最多密钥长度-11
    std::string EncryptPubKey(const std::string_view &str) const;
    // 私钥解密, 每次最多密钥长度-11
    std::string DecryptPriKey(const std::string_view &str) const;
    // 数据签名
    std::string Sign(const std::string_view &str) const;
    // 验证签名
    bool Verify(const std::string_view &datas,
                const std::string_view &sign_datas) const;
private:
    using BIOPtr = std::unique_ptr<BIO, decltype(BIO_free)*>;
    using RSAPtr = std::unique_ptr<RSA, decltype(RSA_free)*>;
    using BNPtr = std::unique_ptr<BIGNUM, decltype(BN_free)*>;
    struct RSAImpl {
        RSA* pub_key_ptr_ = nullptr;
        RSA* pri_key_ptr_ = nullptr;
        ~RSAImpl() {
            if (!pub_key_ptr_) {
                RSA_free(pub_key_ptr_);
            }
            if (!pri_key_ptr_) {
                RSA_free(pri_key_ptr_);
            }
        }
    };
    RSAImpl _impl_;
};

} // namespace mine_openssl

#endif // ! mine_rsa_h