//
// Signature.cpp
//

#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <pssr.h>
#include <osrng.h>
#include <fstream>
#include "Signature.hpp"
#include "Utils.hpp"
#include "Encrypter.hpp"
#include "../Logger.hpp"

namespace network {
    
using namespace CryptoPP;


Signature::Signature(const std::string& filename)
{
    AutoSeededRandomPool rnd;
    InvertibleRSAFunction params;
    params.GenerateRandomWithKeySize(rnd, 3072);

    private_key_ = RSA::PrivateKey(params);
    public_key_ = RSA::PublicKey(params);
}

Signature::~Signature()
{

}

std::string Signature::Sign(const std::string& in)
{
    AutoSeededRandomPool rng;
    RSASS<PSSR, SHA1>::Signer signer(private_key_);
 
    size_t length = signer.MaxSignatureLength();
    SecByteBlock signature(length);

    signer.SignMessage(rng, (const byte*)in.data(), in.size(), signature);
    return std::string(signature.begin(), signature.end());
}

bool Signature::Verify(const std::string& in, const std::string& sign)
{
    RSASS<PSS, SHA1>::Verifier verifier(public_key_);
    return verifier.VerifyMessage((const byte*)in.data(), in.size(),
        (const byte*)sign.data(), sign.size());
}

std::string Signature::GetPublicKey()
{
    ByteQueue queue;
    public_key_.Save(queue);

    size_t length = queue.CurrentSize();
    std::unique_ptr<char[]> outbuf(new char [length]);
    queue.Get((byte*)outbuf.get(), length);

    return std::string((const char*)outbuf.get(), length);
}

void Signature::SetPublicKey(const std::string& in)
{
    if (in.empty()) return;
    ByteQueue queue;
    queue.Put((const byte*)in.data(), in.size());
    public_key_.Load(queue);
}

std::string Signature::GetPrivateKey()
{
    ByteQueue queue;
    private_key_.Save(queue);

    size_t length = queue.CurrentSize();
    std::unique_ptr<char[]> outbuf(new char [length]);
    queue.Get((byte*)outbuf.get(), length);

    return std::string((const char*)outbuf.get(), length);
}

void Signature::SetPrivateKey(const std::string& in)
{
    if (in.empty()) return;
    ByteQueue queue;
    queue.Put((const byte*)in.data(), in.size());
    private_key_.Load(queue);
}

}
