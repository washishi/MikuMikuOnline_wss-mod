//
// Signature.cpp
//

#include <openssl/rand.h>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <openssl/pem.h>
#include <fstream>
#include "Signature.hpp"
#include "Utils.hpp"
#include "Encrypter.hpp"
#include "../Logger.hpp"

namespace network {

const int Signature::RSA_KEY_LENGTH = 256;

Signature::Signature(const std::string& filename)
{
    rsa_key_ = RSA_generate_key(RSA_KEY_LENGTH * 8, RSA_F4, nullptr, nullptr);

    using namespace boost::filesystem;
    path key_file(filename);

    if (key_file.extension() == ".pub") {
        if (exists(key_file)) {
            if (FILE *f = fopen(filename.c_str(), "r")) {
                PEM_read_RSAPublicKey(f, &rsa_key_, nullptr, nullptr);
                fclose(f);
            } else {
                Logger::Error(_T("Cannot open key file: %s"), unicode::ToTString(filename));
            }
        }
    } else {
        if (!exists(filename)) {
            {
                if (FILE* f = fopen(filename.c_str(), "w")) {
                    PEM_write_RSAPrivateKey(f, rsa_key_, nullptr, nullptr, 0, nullptr, nullptr);
                    fclose(f);
                } else {
                    Logger::Error(_T("Cannot write key file: %s"), unicode::ToTString(filename));
                }
            }

            {
                if (FILE* f = fopen((filename + ".pub").c_str(), "w")) {
                    PEM_write_RSAPublicKey(f, rsa_key_);
                    fclose(f);
                } else {
                    Logger::Error(_T("Cannot write key file: %s"), unicode::ToTString(filename));
                }
            }
        } else {
            if (FILE* f = fopen(filename.c_str(), "r")) {
                PEM_read_RSAPrivateKey(f, &rsa_key_, nullptr, nullptr);
                fclose(f);
            } else {
                Logger::Error(_T("Cannot open key file: %s"), unicode::ToTString(filename));
            }
        }
    }
}

Signature::~Signature()
{
    if (rsa_key_) {
        RSA_free(rsa_key_);
    }
}

std::string Signature::Sign(const std::string& in)
{
    char buffer[RSA_KEY_LENGTH];
    unsigned int length;
    auto hash = Encrypter::GetHash(in);
    int result = RSA_sign_ASN1_OCTET_STRING(0,
            (const unsigned char*)hash.data(), hash.size(),
            (unsigned char*)buffer, &length, rsa_key_);

    if (result) {
        return std::string(buffer, length);
    } else {
        Logger::Error(_T("Signature Failed"));
        return std::string();
    }
}

bool Signature::Verify(const std::string& in, const std::string& sign)
{
    auto hash = Encrypter::GetHash(in);
    return RSA_verify_ASN1_OCTET_STRING(0, (const unsigned char*)hash.data(), hash.size(),
            (unsigned char*)sign.data(), sign.size(), rsa_key_);
}

std::string Signature::GetPublicKey()
{
    unsigned char key[RSA_KEY_LENGTH];
    BN_bn2bin(rsa_key_->n, key);
    return std::string((const char*)key, sizeof(key));
}

void Signature::SetPublicKey(const std::string& in)
{
    BN_bin2bn((unsigned char*)in.data(), in.size(), rsa_key_->n);
}

std::string Signature::GetPrivateKey()
{
    unsigned char key[RSA_KEY_LENGTH];
    BN_bn2bin(rsa_key_->d, key);
    return std::string((const char*)key, sizeof(key));
}

void Signature::SetPrivateKey(const std::string& in)
{
    BN_bin2bn((unsigned char*)in.data(), in.size(), rsa_key_->d);
}

}
