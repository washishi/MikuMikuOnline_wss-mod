//
// Signature.hpp
//

#pragma once

#include <string>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/hmac.h>

namespace network {

class Signature {
    public:
        Signature();
        Signature(const std::string& filename);
        ~Signature();

        std::string Sign(const std::string&);
        bool Verify(const std::string& in, const std::string& sign);

        std::string GetPublicKey();
        void SetPublicKey(const std::string&);
        std::string GetPrivateKey();
        void SetPrivateKey(const std::string&);

    public:
        const static int RSA_KEY_LENGTH;
        RSA *rsa_key_;
};

}
