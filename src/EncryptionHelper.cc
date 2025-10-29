#include "EncryptionHelper.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

EncryptionHelper::EncryptionHelper() : privateKey(0), publicKey(0), modulus(0)
{
}

EncryptionHelper::~EncryptionHelper()
{
}

uint64_t EncryptionHelper::gcd(uint64_t a, uint64_t b)
{
    while (b != 0)
    {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

uint64_t EncryptionHelper::modPow(uint64_t base, uint64_t exp, uint64_t mod)
{
    uint64_t result = 1;
    base = base % mod;
    while (exp > 0)
    {
        if (exp % 2 == 1)
        {
            result = (result * base) % mod;
        }
        exp = exp >> 1;
        base = (base * base) % mod;
    }
    return result;
}

uint64_t EncryptionHelper::generatePrime(uint64_t seed)
{
    // Simple prime generation for simulation (not cryptographically secure)
    // Returns a small prime number based on seed
    uint64_t primes[] = {251, 257, 263, 269, 271, 277, 281, 283, 293, 307,
                         311, 313, 317, 331, 337, 347, 349, 353, 359, 367};
    return primes[seed % 20];
}

uint64_t EncryptionHelper::modInverse(uint64_t a, uint64_t m)
{
    // Extended Euclidean Algorithm
    int64_t m0 = m;
    int64_t y = 0, x = 1;

    if (m == 1)
        return 0;

    while (a > 1)
    {
        int64_t q = a / m;
        int64_t t = m;

        m = a % m;
        a = t;
        t = y;

        y = x - q * y;
        x = t;
    }

    if (x < 0)
        x += m0;

    return x;
}

void EncryptionHelper::generateKeyPair(uint64_t seed)
{
    // Generate two distinct primes
    uint64_t p = generatePrime(seed);
    uint64_t q = generatePrime(seed + 1);

    // Ensure p != q
    if (p == q)
    {
        q = generatePrime(seed + 2);
    }

    // Calculate modulus
    modulus = p * q;

    // Calculate Euler's totient
    uint64_t phi = (p - 1) * (q - 1);

    // Choose public exponent (commonly 65537, but we'll use smaller for simulation)
    publicKey = 65537 % phi;
    if (publicKey <= 1)
        publicKey = 17; // Fallback to smaller value

    // Ensure gcd(publicKey, phi) = 1
    while (gcd(publicKey, phi) != 1)
    {
        publicKey++;
    }

    // Calculate private exponent
    privateKey = modInverse(publicKey, phi);
}

std::string EncryptionHelper::getPublicKeyString() const
{
    std::ostringstream oss;
    oss << publicKey << ":" << modulus;
    return oss.str();
}

void EncryptionHelper::setPublicKeyFromString(const std::string &keyStr)
{
    size_t colonPos = keyStr.find(':');
    if (colonPos != std::string::npos)
    {
        publicKey = std::stoull(keyStr.substr(0, colonPos));
        modulus = std::stoull(keyStr.substr(colonPos + 1));
    }
}

std::string EncryptionHelper::encrypt(const std::string &plaintext)
{
    if (!isInitialized())
        return "";

    std::ostringstream oss;

    // Encrypt each character
    for (size_t i = 0; i < plaintext.length(); i++)
    {
        uint64_t charCode = static_cast<uint64_t>(static_cast<unsigned char>(plaintext[i]));
        uint64_t encrypted = modPow(charCode, publicKey, modulus);

        if (i > 0)
            oss << ",";
        oss << encrypted;
    }

    return oss.str();
}

std::string EncryptionHelper::decrypt(const std::string &ciphertext)
{
    if (!isInitialized() || privateKey == 0)
        return "";

    std::string result;
    std::istringstream iss(ciphertext);
    std::string token;

    // Decrypt each encrypted value
    while (std::getline(iss, token, ','))
    {
        if (token.empty())
            continue;

        try
        {
            uint64_t encrypted = std::stoull(token);
            uint64_t decrypted = modPow(encrypted, privateKey, modulus);
            result += static_cast<char>(decrypted);
        }
        catch (...)
        {
            return ""; // Decryption failed
        }
    }

    return result;
}

bool EncryptionHelper::isInitialized() const
{
    return modulus != 0 && publicKey != 0;
}
