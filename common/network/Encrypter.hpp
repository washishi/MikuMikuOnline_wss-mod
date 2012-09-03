//
// Encrypter.hpp
//

#pragma once

#include <string>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/hmac.h>

namespace network {

class Encrypter {
    public:
        Encrypter();
        ~Encrypter();

        std::string Encrypt(const std::string&);
        std::string Decrypt(const std::string&);

        std::string PublicEncrypt(const std::string&);
        std::string PublicDecrypt(const std::string&);

        std::string Sign(const std::string&);
        bool Verify(const std::string& in, const std::string& sign);

        std::string GetPublicKey();
        void SetPublicKey(const std::string&);
        std::string GetPrivateKey();
        void SetPrivateKey(const std::string&);
        void SetPairKey(const std::string&, const std::string&);
        bool CheckKeyPair();

        std::string GetCryptedCommonKey();
        void SetCryptedCommonKey(const std::string&);

        std::string GetPublicKeyFingerPrint();
        static std::string GetHash(const std::string&);
        static std::string GetTrip(const std::string&);

    private:
        std::string GetCommonKey();
        static std::string GetTripHash(const std::string&);

    private:
        const static int COMMON_KEY_LENGTH;
        const static int COMMON_KEY_IV_LENGTH;
        const static int RSA_KEY_LENGTH;
        const static int SHA_LENGTH;
        const static int TRIP_LENGTH;

        // TODO: あとで直す
        std::string common_key_;
        std::string common_key_iv_;

        EVP_CIPHER_CTX ctx_encrypt_,
                       ctx_decrypt_;
        RSA *rsa_key_;
};

}
