#ifndef MINE_HASH_H
#define MINE_HASH_H

#include <openssl/evp.h>

#include <variant>
#include <string_view>
#include <vector>
#include <memory>


namespace mine_openssl {

enum HashLength : std::size_t {
    kSHA256Len = 48,
    kSHA512Len = 64
};

enum class HashType {
    kSHA256Type,
    kSHA512Type
};

class Hash {
public:
    constexpr explicit Hash(const HashType &type)
        : _impl_(HashImpl()) {
        _impl_.hash_type_ = type;
        _impl_.Init();
    }
    virtual ~Hash() = default;
    inline const HashType& type() const noexcept {
        return _impl_.hash_type_;
    }
    /* 添加hash的参数，可以重复添加例如：
     *  hello 
     *  , world */
    void Update(const std::string_view &str) noexcept;
    // 使用了返回值优化
    std::vector<unsigned char> Final() noexcept;

private:
    struct HashImpl {
        mine_openssl::HashType hash_type_;
        using EVPMDCTXPtr = std::unique_ptr<EVP_MD_CTX, decltype(EVP_MD_CTX_free)*>;
        EVPMDCTXPtr md_ctx_ {EVP_MD_CTX_new(), EVP_MD_CTX_free};
        // std::variant<SHA256_CTX, SHA512_CTX> sha_ctx_;
        inline void Init() noexcept {
            using type = HashType;
            switch (hash_type_) {
            case type::kSHA256Type:
                // sha_ctx_ = SHA256_CTX();
                // SHA256_Init(&std::get<SHA256_CTX>(sha_ctx_));
                EVP_DigestInit_ex(md_ctx_.get(), EVP_sha256(), nullptr);
                break;
            case type::kSHA512Type:
                // sha_ctx_ = SHA512_CTX();
                // SHA512_Init(&std::get<SHA512_CTX>(sha_ctx_));
                EVP_DigestInit_ex(md_ctx_.get(), EVP_sha512(), nullptr);
                break;
            default:
                break;
            }
        }
    };
    HashImpl _impl_;
};


} // ! namespace mine_openssl

#endif // ! MINE_HASH_H