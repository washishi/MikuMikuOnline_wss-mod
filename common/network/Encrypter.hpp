//
// Encrypter.hpp
//

#pragma once

#include <string>

#include <modes.h>
#include <aes.h>
#include <rsa.h>

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
        const static int TRIP_LENGTH;

        std::string common_key_;
        std::string common_key_iv_;

        CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption aes_encrypt_;
        CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption aes_decrypt_;

        CryptoPP::RSA::PrivateKey private_key_;
        CryptoPP::RSA::PublicKey public_key_;
};

}
