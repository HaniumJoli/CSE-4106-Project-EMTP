#ifndef ENCRYPTIONHELPER_H_
#define ENCRYPTIONHELPER_H_

#include <string>
#include <cstdint>
#include <vector>

/**
 * Simple RSA-like encryption helper for simulation purposes.
 * This is NOT cryptographically secure - it's a simplified simulation
 * of RSA encryption/decryption for demonstration purposes only.
 */
class EncryptionHelper
{
private:
    uint64_t privateKey;
    uint64_t publicKey;
    uint64_t modulus;

    // Simple modular exponentiation
    uint64_t modPow(uint64_t base, uint64_t exp, uint64_t mod);

    // Simple prime generation for simulation
    uint64_t generatePrime(uint64_t seed);

    // GCD calculation
    uint64_t gcd(uint64_t a, uint64_t b);

    // Modular multiplicative inverse
    uint64_t modInverse(uint64_t a, uint64_t m);

public:
    EncryptionHelper();
    virtual ~EncryptionHelper();

    // Generate RSA key pair
    void generateKeyPair(uint64_t seed);

    // Get public key as string
    std::string getPublicKeyString() const;

    // Set public key from string (for client)
    void setPublicKeyFromString(const std::string &keyStr);

    // Encrypt message with public key
    std::string encrypt(const std::string &plaintext);

    // Decrypt message with private key
    std::string decrypt(const std::string &ciphertext);

    // Check if keys are initialized
    bool isInitialized() const;
};

#endif /* ENCRYPTIONHELPER_H_ */
