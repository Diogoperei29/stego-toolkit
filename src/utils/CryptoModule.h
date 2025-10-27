#ifndef __CRYPTO___MODULE_H_
#define __CRYPTO___MODULE_H_

#include <vector>
#include <string>
#include <cstdint>
#include "ErrorHandler.h"

/**
 * @brief Static class to provide AES-256-CBC encryption and decryption utilities.
 *
 * Uses PBKDF2-HMAC-SHA256 for key derivation with a random salt,
 * AES-256-CBC for encryption/decryption with a random IV,
 * and HMAC-SHA256 for authentication (Encrypt-then-MAC).
 */
class CryptoModule
{
public:
    // Cryptographic constants (made public for capacity calculations)
    static constexpr int SALT_SIZE = 16;                                        // Salt size for PBKDF2 (128 bits)
    static constexpr int IV_SIZE   = 16;                                        // AES block size (128-bit IV)
    static constexpr int KEY_SIZE  = 32;                                        // AES-256 key size (256 bits)
    static constexpr int HMAC_SIZE = 32;                                        // HMAC-SHA256 output size (256 bits)
    static constexpr int MIN_SIZE = SALT_SIZE + IV_SIZE + HMAC_SIZE + 1;        // At least 1 byte of ciphertext
    static constexpr int PBKDF2_ITERATIONS = 10000;                             // PBKDF2 iteration count
    static constexpr int ENCRYPTION_OVERHEAD = SALT_SIZE + IV_SIZE + HMAC_SIZE; // Bytes added during encryption
    
    CryptoModule() = delete;
    
    /**
    * @brief Encrypts data with AES-256-CBC using a password.
    *
    * The output format is: [salt | iv | ciphertext | hmac].
    * HMAC authenticates salt + iv + ciphertext using Encrypt-then-MAC.
    *
    * @param plainData Input plaintext data.
    * @param password Password used for key derivation.
    * @return Result containing encrypted data or error
    */
    static Result<std::vector<uint8_t>> EncryptData(
        const std::vector<uint8_t> &plainData,
        const std::string &password
    );

    /**
    * @brief Decrypts data with AES-256-CBC using a password.
    *
    * Expects input format: [salt | iv | ciphertext | hmac].
    * Verifies HMAC before decryption for authenticated encryption.
    *
    * @param encryptedData Input encrypted data.
    * @param password Password used for key derivation.
    * @return Result containing decrypted data or error
    */
    static Result<std::vector<uint8_t>> DecryptData(
        const std::vector<uint8_t> &encryptedData,
        const std::string &password
    );

private:
    // Helper function to get OpenSSL error string
    static std::string GetOpenSSLError();
};


#endif // __CRYPTO___MODULE_H_