#include "mine_hash.h"

// constexpr mine_openssl::Hash::Hash(const HashType &type) 
//     : _impl_(HashImpl())
// {
//     _impl_.hash_type_ = type;
//     _impl_.Init();
// }

void mine_openssl::Hash::Update(const std::string_view &str) noexcept {
    EVP_DigestUpdate(_impl_.md_ctx_.get(), str.data(), str.size());
}

std::vector<unsigned char> mine_openssl::Hash::Final() noexcept {
    using type = HashType;
    using len = HashLength;
    std::vector<unsigned char> datas;
    unsigned s = datas.size();
    switch (_impl_.hash_type_)
    {
    case type::kSHA256Type: {
        datas.resize(len::kSHA256Len);
        EVP_DigestFinal_ex(_impl_.md_ctx_.get(), datas.data(), &s);
        // SHA256_Final(datas.data(),
        //              &std::get<SHA256_CTX>(_impl_.sha_ctx_));
        break;
    }
    case type::kSHA512Type: {
        datas.resize(len::kSHA512Len);
        EVP_DigestFinal_ex(_impl_.md_ctx_.get(), datas.data(), &s);
        // SHA512_Final(datas.data(),
        //              &std::get<SHA512_CTX>(_impl_.sha_ctx_));
        break;
    }
    default:
        break;
    }
    return datas;
}