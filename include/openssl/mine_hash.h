﻿#ifndef MINE_HASH_H
#define MINE_HASH_H

#include <openssl/evp.h>

#include <variant>
#include <string_view>
#include <vector>
#include <memory>


namespace mine_openssl {

enum HashLength : std::size_t {
    kSHA256Len = SHA256_DIGEST_LENGTH,
    kSHA384Len = SHA384_DIGEST_LENGTH,
    kSHA512Len = SHA512_DIGEST_LENGTH
};
enum class HashType {
    kSHA256Type,
    kSHA384Type,
    kSHA512Type
};

class Hash {
public:
    Hash() = delete;
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
    inline void Update(std::string_view str) noexcept {
        EVP_DigestUpdate(_impl_.md_ctx_.get(), str.data(), str.size());
    }
    // 返回计算结果, 使用后该结构体将重置为初始状态
    inline std::string Final() noexcept {
        using type = HashType;
        using len = HashLength;
        unsigned length = 0;
        switch (_impl_.hash_type_) {
        case type::kSHA256Type: {
            std::array<unsigned char, len::kSHA256Len> datas{0};
            EVP_DigestFinal_ex(_impl_.md_ctx_.get(), datas.data(), &length);
            EVP_DigestInit_ex(_impl_.md_ctx_.get(), EVP_sha256(), nullptr);
            std::array<char, len::kSHA256Len * 2 + 1> result_arr{0};
            for (unsigned i = 0; i != length; ++i) {
                sprintf(&result_arr[i * 2], "%02x", datas[i]);
            }
            std::string result_str(result_arr.data(), length * 2 + 1);
            return result_str;
        }
        case type::kSHA384Type: {
            std::array<unsigned char, len::kSHA384Len> datas{0};
            EVP_DigestFinal_ex(_impl_.md_ctx_.get(), datas.data(), &length);
            EVP_DigestInit_ex(_impl_.md_ctx_.get(), EVP_sha384(), nullptr);
            std::array<char, len::kSHA384Len * 2 + 1> result_arr{0};
            for (unsigned i = 0; i != length; ++i) {
                sprintf(&result_arr[i * 2], "%02x", datas[i]);
            }
            std::string result_str(result_arr.data(), length * 2 + 1);
            return result_str;
        }
        case type::kSHA512Type: {
            std::array<unsigned char, len::kSHA512Len> datas{0};
            EVP_DigestFinal_ex(_impl_.md_ctx_.get(), datas.data(), &length);
            EVP_DigestInit_ex(_impl_.md_ctx_.get(), EVP_sha512(), nullptr);
            std::array<char, len::kSHA512Len * 2 + 1> result_arr{0};
            for (unsigned i = 0; i != length; ++i) {
                sprintf(&result_arr[i * 2], "%02x", datas[i]);
            }
            std::string result_str(result_arr.data(), length * 2 + 1);
            return result_str;
        }
        }
    }

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
            case type::kSHA384Type:
                // sha_ctx_ = SHA256_CTX();
                // SHA256_Init(&std::get<SHA256_CTX>(sha_ctx_));
                EVP_DigestInit_ex(md_ctx_.get(), EVP_sha384(), nullptr);
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