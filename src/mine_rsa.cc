#include "mine_rsa.h"

mine_openssl::MyRSA::MyRSA(const int &n)
    : _impl_(RSAImpl())
{
    RSAPtr rsa_ptr(RSA_new(), RSA_free);
    BNPtr big_num_ptr(BN_new(), BN_free);
    BN_set_word(big_num_ptr.get(), kBigNumW);
    RSA_generate_key_ex(rsa_ptr.get(), kKeyBitsLen * n, 
                        big_num_ptr.get(), nullptr);
    _impl_.pub_key_ptr_ = RSAPublicKey_dup(rsa_ptr.get());
    _impl_.pri_key_ptr_ = RSAPrivateKey_dup(rsa_ptr.get());
}

// constexpr mine_openssl::MyRSA::MyRSA(const std::string_view &pub_key_path,
//                            const std::string_view &pri_key_path)
//     : _impl_(RSAImpl())
// {
//     Init(pub_key_path, pri_key_path);
// }

void mine_openssl::MyRSA::Init(const std::string_view &pub_key_path,
                               const std::string_view &pri_key_path) {
    if (!_impl_.pub_key_ptr_) { // 用于清除之前初始化的数据
        RSA_free(_impl_.pub_key_ptr_);
        _impl_.pub_key_ptr_ = nullptr;
    }
    if (!_impl_.pri_key_ptr_) {
        RSA_free(_impl_.pri_key_ptr_);
        _impl_.pri_key_ptr_ = nullptr;
    }
    // 开始工作
    _impl_.pub_key_ptr_ = RSA_new();
    _impl_.pri_key_ptr_ = RSA_new();
    BIOPtr bio_pub_key_file_ptr(BIO_new_file(pub_key_path.data(), "r"), BIO_free);
    BIOPtr bio_pri_key_file_ptr(BIO_new_file(pri_key_path.data(), "r"), BIO_free);
    PEM_read_bio_RSAPublicKey(bio_pub_key_file_ptr.get(),
        &_impl_.pub_key_ptr_, nullptr, nullptr);
    PEM_read_bio_RSAPrivateKey(bio_pri_key_file_ptr.get(),
        &_impl_.pri_key_ptr_, nullptr, nullptr);
}

void mine_openssl::MyRSA::SaveRSAKey(const std::string_view &pub_key_path,
                                     const std::string_view &pri_key_path) const {
    BIOPtr bio_pub_key_file_ptr(BIO_new_file(pub_key_path.data(), "w"), BIO_free);
    BIOPtr bio_pri_key_file_ptr(BIO_new_file(pri_key_path.data(), "w"), BIO_free);
    PEM_write_bio_RSAPublicKey(bio_pub_key_file_ptr.get(), _impl_.pub_key_ptr_);
    PEM_write_bio_RSAPrivateKey(bio_pri_key_file_ptr.get(), _impl_.pri_key_ptr_,
        nullptr, nullptr, 0, nullptr, nullptr);
}
std::string mine_openssl::MyRSA::EncryptPubKey(const std::string_view &str) const {
    const int len = RSA_size(_impl_.pub_key_ptr_);
    char *datas = new char[len] {0};
    RSA_public_encrypt(str.size(),
        reinterpret_cast<const unsigned char*>(str.data()),
        reinterpret_cast<unsigned char*>(datas),
        _impl_.pub_key_ptr_,
        RSA_PKCS1_PADDING);
    std::string result(datas, len);
    delete[] datas;
    return result;
}

std::string mine_openssl::MyRSA::DecryptPriKey(const std::string_view &str) const {
    const int len = RSA_size(_impl_.pri_key_ptr_);
    char *datas = new char[len] {0};
    RSA_private_decrypt(str.size(),
        reinterpret_cast<const unsigned char*>(str.data()),
        reinterpret_cast<unsigned char*>(datas),
        _impl_.pri_key_ptr_,
        RSA_PKCS1_PADDING);
    std::string result(datas, len);
    delete[] datas;
    return result;
}

std::string mine_openssl::MyRSA::Sign(const std::string_view &str) const {
    const int len = RSA_size(_impl_.pri_key_ptr_);
    char *datas = new char[len] {0};
    unsigned out_len = 0;
    RSA_sign(
        NID_sha512,
        reinterpret_cast<const unsigned char*>(str.data()),
        str.size(),
        reinterpret_cast<unsigned char*>(datas),
        &out_len,
        _impl_.pri_key_ptr_
    );
    std::string result(datas, out_len);
    delete[] datas;
    return result;
}

bool mine_openssl::MyRSA::Verify(const std::string_view &datas,
                                 const std::string_view &sign_datas) const {
    return RSA_verify(
        NID_sha512,
        reinterpret_cast<const unsigned char*>(datas.data()),
        datas.size(),
        reinterpret_cast<const unsigned char*>(sign_datas.data()),
        sign_datas.size(),
        _impl_.pub_key_ptr_
    );
}
