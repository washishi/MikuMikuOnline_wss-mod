//
// Signature.hpp
//

#pragma once

#include <string>
#include <rsa.h>

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
        CryptoPP::RSA::PrivateKey private_key_;
        CryptoPP::RSA::PublicKey public_key_;
};

}
