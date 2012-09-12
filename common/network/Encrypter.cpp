//
// Encrypter.cpp
//

#include <boost/format.hpp>
#include <sha.h>
#include <osrng.h>
#include "Encrypter.hpp"
#include "Utils.hpp"

#ifdef _WIN32
#pragma comment(lib, "cryptlib.lib")
#endif

namespace network {

const int Encrypter::TRIP_LENGTH = 20;

using namespace CryptoPP;

Encrypter::Encrypter()
{
    AutoSeededRandomPool rnd;
    
    byte common_key[AES::DEFAULT_KEYLENGTH];
    byte common_key_iv[AES::BLOCKSIZE];
    rnd.GenerateBlock(common_key, sizeof(common_key));
    rnd.GenerateBlock(common_key_iv, sizeof(common_key_iv));

    common_key_ = std::string((const char*)common_key, sizeof(common_key));
    common_key_iv_ = std::string((const char*)common_key_iv, sizeof(common_key_iv));

    aes_encrypt_.SetKeyWithIV(common_key, sizeof(common_key), common_key_iv);
    aes_decrypt_.SetKeyWithIV(common_key, sizeof(common_key), common_key_iv);

    InvertibleRSAFunction params;
    params.GenerateRandomWithKeySize(rnd, 3072);

    private_key_ = RSA::PrivateKey(params);
    public_key_ = RSA::PublicKey(params);
}

Encrypter::~Encrypter()
{
}

std::string Encrypter::Encrypt(const std::string& in)
{
     std::unique_ptr<char[]> outbuf(new char [in.size()]);
     aes_encrypt_.ProcessData((byte*)outbuf.get(), (const byte*)in.data(), in.size());
     std::string out((const char*)outbuf.get(), in.size());
     return out;
}

std::string Encrypter::Decrypt(const std::string& in)
{
     std::unique_ptr<char[]> outbuf(new char [in.size()]);
     aes_decrypt_.ProcessData((byte*)outbuf.get(), (const byte*)in.data(), in.size());
     std::string out((const char*)outbuf.get(), in.size());
     return out;
}

std::string Encrypter::GetPublicKey()
{
    ByteQueue queue;
    public_key_.Save(queue);

    size_t length = queue.CurrentSize();
    std::unique_ptr<char[]> outbuf(new char [length]);
    queue.Get((byte*)outbuf.get(), length);

    return std::string((const char*)outbuf.get(), length);
}

void Encrypter::SetPublicKey(const std::string& in)
{
    if (in.empty()) return;
    ByteQueue queue;
    queue.Put((const byte*)in.data(), in.size());
    public_key_.Load(queue);
}

std::string Encrypter::GetPrivateKey()
{
    ByteQueue queue;
    private_key_.Save(queue);

    size_t length = queue.CurrentSize();
    std::unique_ptr<char[]> outbuf(new char [length]);
    queue.Get((byte*)outbuf.get(), length);

    return std::string((const char*)outbuf.get(), length);
}

void Encrypter::SetPrivateKey(const std::string& in)
{
    if (in.empty()) return;
    ByteQueue queue;
    queue.Put((const byte*)in.data(), in.size());
    private_key_.Load(queue);
}

void Encrypter::SetPairKey(const std::string& pub, const std::string& pri)
{
    SetPublicKey(pub);
    SetPrivateKey(pri);
}

std::string Encrypter::GetCryptedCommonKey()
{
    return PublicEncrypt(GetCommonKey());
}

void Encrypter::SetCryptedCommonKey(const std::string& in)
{
    std::string key = PublicDecrypt(in);
    common_key_ = key.substr(0, AES::DEFAULT_KEYLENGTH);
    common_key_iv_ = key.substr(AES::DEFAULT_KEYLENGTH, AES::BLOCKSIZE);

    aes_encrypt_.SetKeyWithIV((const byte*)common_key_.data(), common_key_.size(), (const byte*)common_key_iv_.data());
    aes_decrypt_.SetKeyWithIV((const byte*)common_key_.data(), common_key_.size(), (const byte*)common_key_iv_.data());
}

std::string Encrypter::PublicEncrypt(const std::string& in)
{
    AutoSeededRandomPool rng;
    RSAES_OAEP_SHA_Encryptor encryptor(public_key_);

    // Now that there is a concrete object, we can validate
    assert(0 != encryptor.FixedMaxPlaintextLength());
    assert(in.size() <= encryptor.FixedMaxPlaintextLength());

    // Create cipher text space
    size_t ecl = encryptor.CiphertextLength(in.size());
    assert(0 != ecl);
    SecByteBlock ciphertext(ecl);

    encryptor.Encrypt(rng, (const byte*)in.data(), in.size(), ciphertext);

    return std::string(ciphertext.begin(), ciphertext.end());
}

std::string Encrypter::PublicDecrypt(const std::string& in)
{
    AutoSeededRandomPool rng;
    RSAES_OAEP_SHA_Decryptor decryptor(private_key_);

    // Now that there is a concrete object, we can check sizes
    assert(0 != decryptor.FixedCiphertextLength());
    assert(in.size() <= decryptor.FixedCiphertextLength());

    // Create recovered text space
    size_t dpl = decryptor.MaxPlaintextLength(in.size());
    assert(0 != dpl);
    SecByteBlock recovered(dpl);

    DecodingResult result = decryptor.Decrypt(rng, (const byte*)in.data(), in.size(), recovered);

    return std::string(recovered.begin(), recovered.end());
}

std::string Encrypter::GetPublicKeyFingerPrint()
{
    return GetHash(GetPublicKey());
}

std::string Encrypter::GetHash(const std::string& in)
{
    std::unique_ptr<byte[]> outbuf(new byte [64]);
    CryptoPP::SHA512().CalculateDigest(outbuf.get(), (const byte*)in.data(), in.size());

    return std::string((char*)outbuf.get(), 64);
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
    return GetHash(in);
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

