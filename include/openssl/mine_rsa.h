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
    explicit MyRSA(const int &n = 1)
        : _impl_(RSAImpl()) {
        RSAPtr rsa_ptr(RSA_new(), RSA_free);
        BNPtr big_num_ptr(BN_new(), BN_free);
        BN_set_word(big_num_ptr.get(), kBigNumW);
        RSA_generate_key_ex(rsa_ptr.get(), kKeyBitsLen * n, 
                            big_num_ptr.get(), nullptr);
        _impl_.pub_key_ptr_ = RSAPublicKey_dup(rsa_ptr.get());
        _impl_.pri_key_ptr_ = RSAPrivateKey_dup(rsa_ptr.get());
    }
    // 用于初始化以拥有的密钥
    // 用于解决bool匹配问题
    explicit MyRSA(const char *pub_key_path,
                   const char *pri_key_path)
        : _impl_(RSAImpl()) {
        Init(pub_key_path, pri_key_path);
    }
    explicit MyRSA(std::string_view pub_key_path,
                   std::string_view pri_key_path)
        : _impl_(RSAImpl()) {
        Init(pub_key_path, pri_key_path);
    }
    explicit MyRSA(std::string_view file_name, bool is_pubkey)
        : _impl_(RSAImpl()) {
        InitImpl(file_name, is_pubkey);
    }
    ~MyRSA() = default;
    // 生成密钥对
    inline void Init(std::string_view pub_key_path,
                     std::string_view pri_key_path) {
        if (!_impl_.pub_key_ptr_) { // 用于清除之前初始化的数据
            RSA_free(_impl_.pub_key_ptr_);
            _impl_.pub_key_ptr_ = nullptr;
        }
        if (!_impl_.pri_key_ptr_) {
            RSA_free(_impl_.pri_key_ptr_);
            _impl_.pri_key_ptr_ = nullptr;
        }
        // 开始工作
        InitImpl(pub_key_path, true);
        InitImpl(pri_key_path, false);
    }
    // 储存密钥对
    inline void SaveRSAKey(std::string_view pub_key_path,
                           std::string_view pri_key_path) const {
        BIOPtr bio_pub_key_file_ptr(BIO_new_file(pub_key_path.data(), "w"), BIO_free);
        BIOPtr bio_pri_key_file_ptr(BIO_new_file(pri_key_path.data(), "w"), BIO_free);
        PEM_write_bio_RSAPublicKey(bio_pub_key_file_ptr.get(), _impl_.pub_key_ptr_);
        PEM_write_bio_RSAPrivateKey(bio_pri_key_file_ptr.get(), _impl_.pri_key_ptr_,
            nullptr, nullptr, 0, nullptr, nullptr);
    }
    // 公钥加密, 每次最多密钥长度-11
    inline std::string EncryptPubKey(std::string_view str) const {
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
    // 私钥解密, 每次最多密钥长度-11
    inline std::string DecryptPriKey(std::string_view str) const {
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
    // 数据签名
    std::string Sign(std::string_view str) const {
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
    // 验证签名
    bool Verify(std::string_view datas,
                std::string_view sign_datas) const {
        return RSA_verify(
            NID_sha512,
            reinterpret_cast<const unsigned char*>(datas.data()),
            datas.size(),
            reinterpret_cast<const unsigned char*>(sign_datas.data()),
            sign_datas.size(),
            _impl_.pub_key_ptr_
        );
    }
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
    inline void InitImpl(std::string_view file_name, bool is_pubkey) {
        if (is_pubkey) {
            _impl_.pub_key_ptr_ = RSA_new();
            BIOPtr bio_pub_key_file_ptr(BIO_new_file(file_name.data(), "r"), BIO_free);
            PEM_read_bio_RSAPublicKey(bio_pub_key_file_ptr.get(),
                &_impl_.pub_key_ptr_, nullptr, nullptr);
        } else {
            _impl_.pri_key_ptr_ = RSA_new();
            BIOPtr bio_pri_key_file_ptr(BIO_new_file(file_name.data(), "r"), BIO_free);
            PEM_read_bio_RSAPrivateKey(bio_pri_key_file_ptr.get(),
                &_impl_.pri_key_ptr_, nullptr, nullptr);
        }
    }
    RSAImpl _impl_;
};

} // namespace mine_openssl

#endif // ! mine_rsa_h
