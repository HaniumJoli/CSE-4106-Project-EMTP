#ifndef ENCRYPTIONHELPER_H_
#define ENCRYPTIONHELPER_H_

#include <string>
#include <cstdint>
#include <vector>

class EncryptionHelper
{
private:
    uint64_t privateKey;
    uint64_t publicKey;
    uint64_t modulus;

    uint64_t modPow(uint64_t base, uint64_t exp, uint64_t mod);

    uint64_t generatePrime(uint64_t seed);

    uint64_t gcd(uint64_t a, uint64_t b);

    uint64_t modInverse(uint64_t a, uint64_t m);

public:
    EncryptionHelper();
    virtual ~EncryptionHelper();

    void generateKeyPair(uint64_t seed);

    std::string getPublicKeyString() const;

    void setPublicKeyFromString(const std::string &keyStr);

    // Encrypt with public key
    std::string encrypt(const std::string &plaintext);

    // Decrypt with private key
    std::string decrypt(const std::string &ciphertext);

    bool isInitialized() const;
};

#endif /* ENCRYPTIONHELPER_H_ */
