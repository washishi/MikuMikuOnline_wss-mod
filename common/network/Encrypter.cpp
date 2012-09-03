//
// Encrypter.cpp
//

#include <openssl/rand.h>
#include <boost/format.hpp>
#include "Encrypter.hpp"
#include "Utils.hpp"

#ifdef _WIN32
#include <openssl/applink.c>
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")
#endif

namespace network {

// フィンガープリント用ハッシュで用いるHMACパスワード
// ** 警告 ** この値を変更すると、変更以前に登録したフィンガープリントが全て無効になります。
const std::string SHA_HMAC_KEY("TXpnM01XVTJNemRrTlRSa1pqUXlZVEUxWm1Rd1lqVmhNelkxTWpFMVpHTQ==");

// トリップ用ハッシュで用いるHMACパスワード
// ** 警告 ** この値を変更すると、サーバーで生成されるトリップが変更されます。
const std::string TRIP_SHA_HMAC_KEY("RfNmIE9QCk7CiNlbmDAodmRpZgojaWRU5TU0xf9pZZuZGVTk9fSURFQpjs==");

const int Encrypter::COMMON_KEY_LENGTH = 16;
const int Encrypter::COMMON_KEY_IV_LENGTH = 16;
const int Encrypter::RSA_KEY_LENGTH = 256;
const int Encrypter::SHA_LENGTH = 64;
const int Encrypter::TRIP_LENGTH = 20;

Encrypter::Encrypter()
{
    unsigned char common_key[COMMON_KEY_LENGTH];
    unsigned char common_key_iv[COMMON_KEY_IV_LENGTH];
    RAND_bytes(common_key, sizeof(common_key));
    RAND_bytes(common_key_iv, sizeof(common_key_iv));

    common_key_ = std::string((char*)common_key, sizeof(common_key));
    common_key_iv_ = std::string((char*)common_key_iv, sizeof(common_key_iv));

    EVP_CIPHER_CTX_init(&ctx_encrypt_);
    EVP_CIPHER_CTX_init(&ctx_decrypt_);
    EVP_EncryptInit_ex(&ctx_encrypt_, EVP_aes_128_cbc(),
        NULL, (const unsigned char*)common_key_.data(), (const unsigned char*)common_key_iv_.data());

    EVP_DecryptInit_ex(&ctx_decrypt_, EVP_aes_128_cbc(),
        NULL, (const unsigned char*)common_key_.data(), (const unsigned char*)common_key_iv_.data());

    rsa_key_ = RSA_generate_key(RSA_KEY_LENGTH * 8, RSA_F4, NULL, NULL);
}

Encrypter::~Encrypter()
{
    EVP_CIPHER_CTX_cleanup(&ctx_encrypt_);
    EVP_CIPHER_CTX_cleanup(&ctx_decrypt_);
    RSA_free(rsa_key_);
}

std::string Encrypter::Encrypt(const std::string& in)
{
    // TODO: 適切なバッファ長を計算する
     unsigned char* outbuf = new unsigned char[2048];
     int outlen, tmplen;

     EVP_EncryptInit_ex(&ctx_encrypt_, NULL, NULL, NULL, NULL);
     if(!EVP_EncryptUpdate(&ctx_encrypt_, outbuf, &outlen, (unsigned char*)in.data(), in.size()) ||
             !EVP_EncryptFinal_ex(&ctx_encrypt_, outbuf + outlen, &tmplen)) {

         return std::string();
     }

     outlen += tmplen;

     std::string out((const char*)outbuf, outlen);
     delete[] outbuf;

     return out;
}

std::string Encrypter::Decrypt(const std::string& in)
{
    // TODO: 適切なバッファ長を計算する
     unsigned char* outbuf = new unsigned char[2048];
     int outlen, tmplen;

     EVP_DecryptInit_ex(&ctx_decrypt_, NULL, NULL, NULL, NULL);
     if(!EVP_DecryptUpdate(&ctx_decrypt_, outbuf, &outlen, (unsigned char*)in.data(), in.size()) ||
             !EVP_DecryptFinal_ex(&ctx_decrypt_, outbuf + outlen, &tmplen)) {

         return std::string();
     }

     outlen += tmplen;

     std::string out((const char*)outbuf, outlen);
     delete[] outbuf;

     return out;
}

std::string Encrypter::GetPublicKey()
{
    unsigned char key[RSA_KEY_LENGTH];
    BN_bn2bin(rsa_key_->n, key);
    return std::string((const char*)key, sizeof(key));
}

void Encrypter::SetPublicKey(const std::string& in)
{
    BN_bin2bn((unsigned char*)in.data(), in.size(), rsa_key_->n);
}

std::string Encrypter::GetPrivateKey()
{
    unsigned char key[RSA_KEY_LENGTH];
    BN_bn2bin(rsa_key_->d, key);
    return std::string((const char*)key, sizeof(key));
}

void Encrypter::SetPrivateKey(const std::string& in)
{
    BN_bin2bn((unsigned char*)in.data(), in.size(), rsa_key_->d);
}

void Encrypter::SetPairKey(const std::string& pub, const std::string& pri)
{
    BN_bin2bn((unsigned char*)pub.data(), pub.size(), rsa_key_->n);
    BN_bin2bn((unsigned char*)pri.data(), pri.size(), rsa_key_->d);
}

std::string Encrypter::GetCryptedCommonKey()
{
    return PublicEncrypt(GetCommonKey());
}

void Encrypter::SetCryptedCommonKey(const std::string& in)
{
    std::string key = PublicDecrypt(in);
    common_key_ = key.substr(0, COMMON_KEY_LENGTH);
    common_key_iv_ = key.substr(COMMON_KEY_LENGTH, COMMON_KEY_IV_LENGTH);

    EVP_EncryptInit_ex(&ctx_encrypt_, EVP_aes_128_cbc(), NULL,
        (const unsigned char*)common_key_.data(), (const unsigned char*)common_key_iv_.data());

    EVP_DecryptInit_ex(&ctx_decrypt_, EVP_aes_128_cbc(), NULL,
        (const unsigned char*)common_key_.data(), (const unsigned char*)common_key_iv_.data());
}

std::string Encrypter::PublicEncrypt(const std::string& in)
{
    int outlen = RSA_size(rsa_key_);
    unsigned char* outbuf = new unsigned char[outlen];

    RSA_public_encrypt(in.size(), (unsigned char *)in.data(),
            outbuf, rsa_key_, RSA_PKCS1_OAEP_PADDING);

    std::string out((const char*)outbuf, outlen);
    delete[] outbuf;

    return out;
}

std::string Encrypter::PublicDecrypt(const std::string& in)
{
    int outlen = RSA_size(rsa_key_);
    unsigned char* outbuf = new unsigned char[outlen];

    RSA_private_decrypt(in.size(), (unsigned char *)in.data(),
            outbuf, rsa_key_, RSA_PKCS1_OAEP_PADDING);

    std::string out((const char*)outbuf, outlen);
    delete[] outbuf;

    return out;
}

std::string Encrypter::GetPublicKeyFingerPrint()
{
    return GetHash(GetPublicKey());
}

std::string Encrypter::GetHash(const std::string& in)
{
    int outlen = SHA_LENGTH;
    unsigned char* outbuf = new unsigned char[outlen];

    static const std::string key = Utils::Base64Decode(SHA_HMAC_KEY).data();
    HMAC(EVP_sha512(), key.data(), key.size(), (unsigned char*)in.data(), in.size(), outbuf, NULL);

    std::string out((char*)outbuf, outlen);
    delete[] outbuf;

    return out;
}

std::string Encrypter::GetTrip(const std::string& in)
{
    static const unsigned char trip_chars[] =
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            ".:;@#$%&_()=*{}~+-!?[]()^/";

    int pattern_size = sizeof(trip_chars) - 1;

    auto buffer = GetTripHash(in).substr(0, TRIP_LENGTH);
    std::string out;

    for (auto it = buffer.begin(); it != buffer.end(); ++it) {
        const unsigned char & c = *it;
        out += trip_chars[c % pattern_size];
    }

    return out;
}

std::string Encrypter::GetTripHash(const std::string& in)
{
    int outlen = SHA_LENGTH;
    unsigned char* outbuf = new unsigned char[outlen];

    static const std::string key = Utils::Base64Decode(TRIP_SHA_HMAC_KEY).data();
    HMAC(EVP_sha512(), key.data(), key.size(), (unsigned char*)in.data(), in.size(), outbuf, NULL);

    std::string out((char*)outbuf, outlen);
    delete[] outbuf;

    return out;
}

bool Encrypter::CheckKeyPair()
{
    // 復号化できるかチェック
    std::string test_data("test");
    bool result = (PublicDecrypt(PublicEncrypt(test_data)).substr(0, 4) == test_data);
    return result;
}

std::string Encrypter::GetCommonKey()
{
    return common_key_ + common_key_iv_;
}

}

